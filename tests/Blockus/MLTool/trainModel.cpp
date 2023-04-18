#include <array>
#include <iostream>
#include <filesystem>
#include <windows.h>

#include "AI/blockusAI/BlokusAI.h"
#include "ML/Dataset.h"
#include "ML/NetworkDef.h"

using namespace torch;

std::string getDatasetPath(string _outputFolder, string _datasetBaseName, u32 _index);
u32 findNumDatasetOnDisk(string _outputFolder, string _datasetBaseName);

const u32 batchSize = 32;

float evalPrecision(torch::Tensor prediction, torch::Tensor labels)
{
    prediction = torch::round(prediction);
    auto p = torch::mean(torch::abs(prediction - labels));
    
    return p.item<float>();
}

std::pair<float,float> evalMeanLoss(const blokusAI::Dataset::Batches& _testset, std::shared_ptr<blokusAI::BlokusNet> _net, torch::Tensor _weightsTensor)
{
    torch::NoGradGuard _{};
    float averageLoss = 0;
    float averagePrecision = {};
    u32 numElems = 0;
    for (auto& [batch_data, batch_labels] : _testset)
    {
        torch::Tensor prediction = _net->forward(batch_data);
        torch::Tensor loss = torch::binary_cross_entropy(prediction, batch_labels, _weightsTensor);

        numElems += batch_data.sizes()[0];
        averageLoss += loss.item<float>() * batch_data.sizes()[0];
        averagePrecision += evalPrecision(prediction, batch_labels) * batch_data.sizes()[0];
    }

    return { averageLoss / numElems, averagePrecision / numElems };
}

int trainModel(string _model, string _datasetFolder, string _datasetBaseName, string _testsetName, string _inModelPath, string _outModelPath, u32 _datasetIndex, float _lr, uvec2 _turnRange, u32 _turnOffset, bool _useCluster, bool _autoLr, bool _autoExit)
{
    if(!_autoExit)
        system("pause");

    blokusAI::initBlokusAI();

    using namespace torch::indexing;
    _inModelPath += "_" + _model + "_" + std::to_string(_turnOffset) + ".pt";
    _outModelPath += "_" + _model + "_" + std::to_string(_turnOffset) + ".pt";

    std::cout << "Train model with param : " << std::endl;
    std::cout << "Model : " << _model << std::endl;
    std::cout << "Folder : " << _datasetFolder << std::endl;
    std::cout << "Basename : " << _datasetBaseName << std::endl;
    std::cout << "Test set : " << _testsetName << std::endl;
    std::cout << "Input model : " << _inModelPath << std::endl;
    std::cout << "Ouput model : " << _outModelPath << std::endl;
    std::cout << "Offset= " << _datasetIndex <<  " Lr=" << _lr << " Range=[" << _turnRange.x << "," << _turnRange.y << "] Cluster=" << _useCluster << std::endl;

    u32 numDataset = findNumDatasetOnDisk(_datasetFolder, _datasetBaseName);
    std::cout << "Num dataset : " << numDataset << std::endl;

    std::shared_ptr<blokusAI::BlokusNet> net;
    if(_model == "jojo")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_Jojo, _useCluster ? 4 : 2);
    else if (_model == "twolayers")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_TwoLayers, _useCluster ? 4 : 2);
    else if (_model == "simplecnn")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_SimpleCnn, _useCluster ? 4 : 2);
    else if (_model == "simplecnn2")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_SimpleCnn2, _useCluster ? 4 : 2);
    else if (_model == "cnn1")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_Cnn1, _useCluster ? 4 : 2);
    else if(_model == "baseline")
        net = std::make_shared<blokusAI::BlokusNet>(blokusAI::BlokusNet::Model::Model_Baseline, _useCluster ? 4 : 2);
    else
    {
        std::cout << _model << " is not recognized." << std::endl;
        return -1;
    }

    if (std::filesystem::exists(_inModelPath))
    {
        torch::load(net, _inModelPath);
        std::cout << "Load model " << _inModelPath << std::endl;
    }

    std::unique_ptr<blokusAI::Dataset::Batches> testBatches;
    torch::Tensor weightTestset;
    if (_testsetName.empty() == false)
    {
        std::string testsetPath = getDatasetPath(_datasetFolder, _testsetName, 0);
        if (std::filesystem::exists(testsetPath))
        {
            blokusAI::Dataset dataset;
            dataset.read(testsetPath);
            dataset.shuffle();
            weightTestset = dataset.computeWeight();
            testBatches = std::make_unique<blokusAI::Dataset::Batches>(dataset.constructTensors(batchSize, _turnRange, _turnOffset, _useCluster));
            auto [loss, p] = evalMeanLoss(*testBatches, net, weightTestset);
            std::cout << "Test set size : " << testBatches->size() << " loss=" << loss << " p=" << p << std::endl;
        }
    }

    bool canAutoExit = false;
    while(1)
    {
        using OptimizerType = torch::optim::AdamW;
        std::unique_ptr<OptimizerType> optimizer = std::make_unique<OptimizerType>(net->parameters(), _lr);
        float prevAvgTestLossOverDataset = 1e6;

        for (u32 i = _datasetIndex; i < numDataset; ++i)
        {
            blokusAI::Dataset dataset;
            dataset.read(getDatasetPath(_datasetFolder, _datasetBaseName, i));
            auto weights = dataset.computeWeight();

            auto batches = dataset.constructTensors(batchSize, _turnRange, _turnOffset, _useCluster);
            float averageLoss = 0, averageLossOverDataset = 0;

            u32 batchIndex = 0;
            for (auto& [batch_data, batch_labels] : batches)
            {
                optimizer->zero_grad();
                
                torch::Tensor prediction = net->forward(batch_data);
                torch::Tensor loss = torch::binary_cross_entropy(prediction, batch_labels, weights);
                loss.backward();
                optimizer->step();

                averageLoss += loss.item<float>();
                averageLossOverDataset += loss.item<float>();

                constexpr u32 reportingInterval = 200;
                if (++batchIndex % reportingInterval == 0)
                {
                    std::cout << "Dataset: " << i << " | Batch: " << batchIndex << "/" << batches.size() << " | Loss: " << averageLoss / reportingInterval << std::endl;

                    optimizer->zero_grad();
                    averageLoss = 0;
                }
            }

            averageLossOverDataset = averageLossOverDataset / batches.size();
            
            // Dataset have been processed
            {
                auto [testsetLoss, testsetPrecision] = testBatches ? evalMeanLoss(*testBatches, net, weightTestset) : std::make_pair(0.f, 0.f);
                std::cout << "Testset: loss=" << testsetLoss << " p=" << 1.f - testsetPrecision << std::endl;

                if (testsetLoss > prevAvgTestLossOverDataset)
                {
                    if (_autoLr)
                    {
                        _lr /= 4;
                        std::cout << "Avg loss on testset has decreased over this dataset, new Lr: " << _lr << std::endl;
                    }
                    else if (_autoExit)
                    {
                        if(canAutoExit)
                            return 0;
                    }
                    else
                    {
                        std::cout << "Avg loss on testset has decreased over this dataset, would you like to continue :";
                        int continu = 0; std::cin >> continu;
                        if (!continu)
                            return 0;

                        std::cout << "Divide learning rate by : ";
                        float devLr = 1; std::cin >> devLr;
                        _lr /= devLr;
                    }

                    optimizer = std::make_unique<OptimizerType>(net->parameters(), _lr);
                }
                prevAvgTestLossOverDataset = testsetLoss;

                // Serialize model periodically as a checkpoint.
                std::cout << "Save " << _outModelPath << std::endl;
                torch::save(net, _outModelPath);

                std::filesystem::path extraInfoPath = _outModelPath;
                extraInfoPath.replace_filename("log_" + extraInfoPath.filename().string());
                extraInfoPath.replace_extension("txt");
                std::ofstream extraInfoFile(extraInfoPath, std::ios_base::app);
                extraInfoFile << "Dataset=" << getDatasetPath(_datasetFolder, _datasetBaseName, i) << " Lr=" << _lr <<
                                 " Loss=" << averageLossOverDataset <<
                                 " TestLoss=" << testsetLoss <<
                                 " TestP=" << 1.f - testsetPrecision << std::endl;
            }
        }

        _datasetIndex = 0;
        canAutoExit = true;
    }

    return 0;
}

int shuffleDataset(string _datasetFolder, string _datasetBaseName, u32 _numDatasetOut)
{
    blokusAI::initBlokusAI();
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

    u32 numDataPerSet = dataset.count() / _numDatasetOut;
    for (u32 i = 0; i < _numDatasetOut; ++i)
    {
        dataset.serialize(getDatasetPath(_datasetFolder, _datasetBaseName, i), i * numDataPerSet, numDataPerSet);
    }

    return 0;
}