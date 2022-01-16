#include <array>
#include <iostream>
#include <filesystem>
#include <windows.h>

#include "AI/BlokusAI.h"
#include "ML/Dataset.h"
#include "ML/NetworkDef.h"

using namespace torch;

std::string getDatasetPath(string _outputFolder, string _datasetBaseName, u32 _index);
u32 findNumDatasetOnDisk(string _outputFolder, string _datasetBaseName);

int trainModel(string _datasetFolder, string _datasetBaseName, string _inModelPath, string _outModelPath, u32 _datasetIndex, float _lr, uvec2 _turnRange, bool _useCluster)
{
    blokusAI::initBlokusAI();

    using namespace torch::indexing;

    std::cout << "Train model with param : " << std::endl;
    std::cout << "Folder : " << _datasetFolder << std::endl;
    std::cout << "Basename : " << _datasetBaseName << std::endl;
    std::cout << "Input model : " << _inModelPath << std::endl;
    std::cout << "Ouput model : " << _outModelPath << std::endl;
    std::cout << "Offset= " << _datasetIndex <<  " Lr=" << _lr << " Range=[" << _turnRange.x << "," << _turnRange.y << "] Cluster=" << _useCluster << std::endl;

    u32 numDataset = findNumDatasetOnDisk(_datasetFolder, _datasetBaseName);
    std::cout << "Num dataset : " << numDataset << std::endl;

    auto net = std::make_shared<blokusAI::NetJojo>(_useCluster ? 4 : 2, false);
    // torch::NoGradGuard no_grad;

    if(std::filesystem::exists(_inModelPath))
        torch::load(net, _inModelPath);

    float labelWeights[2] = { 0.25, 0.5 };
    torch::Tensor weightsTensor = torch::from_blob(labelWeights, { 2 });

    while (1)
    {
        for (u32 i = _datasetIndex; i < numDataset; ++i)
        {
            blokusAI::Dataset dataset;
            dataset.read(getDatasetPath(_datasetFolder, _datasetBaseName, i));

            auto batches = dataset.constructTensors(16, _turnRange, _useCluster);
            torch::optim::SGD optimizer(net->parameters(), _lr);

            float averageLoss = 0, averageLossOverDataset = 0;
            u32 batchIndex = 0;
            for (auto& [batch_data, batch_labels] : batches)
            {
                optimizer.zero_grad();
                torch::Tensor prediction = net->forward(batch_data);
                torch::Tensor loss = net->outputScore ? torch::mse_loss(prediction.flatten(), batch_labels) : torch::binary_cross_entropy(prediction, batch_labels, weightsTensor);
                loss.backward();
                optimizer.step();

                averageLoss += loss.item<float>();
                averageLossOverDataset += loss.item<float>();

                constexpr u32 reportingInterval = 100;
                if (++batchIndex % reportingInterval == 0)
                {
                    std::cout << "Dataset: " << i << " | Batch: " << batchIndex << "/" << batches.size()
                              << " | Loss: " << averageLoss / reportingInterval << std::endl;

                    averageLoss = 0;
                }
            }

            // Serialize model periodically as a checkpoint.
            std::cout << "Save " << _outModelPath << std::endl;
            torch::save(net, _outModelPath);
            {
                std::filesystem::path extraInfoPath = _outModelPath;
                extraInfoPath.replace_extension("txt");
                std::ofstream extraInfoFile(extraInfoPath, std::ios_base::trunc);
                extraInfoFile << "Loss=" << averageLossOverDataset / batches.size() << std::endl;
                extraInfoFile << "dataset=" << getDatasetPath(_datasetFolder, _datasetBaseName, i) << std::endl;
            }
        }

        _datasetIndex = 0;
    }

    return 0;
}

int shuffleDataset(string _datasetFolder, string _datasetBaseName)
{
    u32 numDataset = findNumDatasetOnDisk(_datasetFolder, _datasetBaseName);

    blokusAI::Dataset dataset;
    {
        blokusAI::Dataset tmp;
        for (u32 i = 0; i < numDataset; ++i)
        {
            tmp.read(getDatasetPath(_datasetFolder, _datasetBaseName, i));
            dataset.merge(std::move(tmp));
        }
    }

    dataset.shuffle();

    u32 numDataPerSet = dataset.count() / numDataset;
    for (u32 i = 0; i < numDataset; ++i)
    {
        dataset.serialize(getDatasetPath(_datasetFolder, _datasetBaseName, i), i * numDataPerSet, numDataPerSet);
    }

    return 0;
}