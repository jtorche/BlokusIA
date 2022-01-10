#include <array>
#include <iostream>
#include <filesystem>
#include <windows.h>

#include "AI/BlokusAI.h"
#include "ML/Dataset.h"

using namespace torch;

struct NetJojo : torch::nn::Module 
{
    NetJojo(u32 _inChannelCount, bool _outputScore) : outputScore{ _outputScore }
    {
        conv1 = register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 5).padding(2).stride(2)));
        conv2 = register_module("conv2", nn::Conv2d(nn::Conv2dOptions(64, 128, 3).padding(1)));
        maxPool = register_module("maxPool", nn::MaxPool2d(nn::MaxPool2dOptions(2)));
        conv3 = register_module("conv3", nn::Conv2d(nn::Conv2dOptions(128, 256, 3).padding(1)));
        // batch_norm = register_module("batch_norm", nn::BatchNorm2d());
        fully1 = register_module("fully1", nn::Linear(6400, 512));
        fully2 = register_module("fully2", nn::Linear(512, _outputScore ? 1 : 2));
    }

    // Implement the Net's algorithm.
    torch::Tensor forward(torch::Tensor x) 
    {
        x = torch::relu(conv1->forward(x));
        x = torch::relu(conv2->forward(x));
        //x = maxPool->forward(x);
        x = torch::relu(conv3->forward(x));
        x = maxPool->forward(x);
        x = x.flatten(1);
        x = torch::relu(fully1->forward(x));
        x = fully2->forward(x);

        if (outputScore)
        {
            // the result is a score so we can use the linear outpput, however to help a bit we clamp the value as  it is always between 0-1
            x = x.clamp(0, 1);
        }
        else
        {
            x = torch::sigmoid(x);
        }

        return x;
    }

    bool outputScore;
    torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr, conv3 = nullptr;
    torch::nn::MaxPool2d maxPool = nullptr;
    torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    nn::BatchNorm2d batch_norm = nullptr;
};

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

    auto net = std::make_shared<NetJojo>(_useCluster ? 4 : 2, false);
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