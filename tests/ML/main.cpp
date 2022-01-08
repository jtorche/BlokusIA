#include <array>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include "ML/cxxopts.h"

#include "ML/GameGenerator.h"

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/Dummy_AI.h"
#include "AI/AlonePlayer_AI.h"

using namespace blokusAI;
using namespace torch;

struct NetJojo : torch::nn::Module 
{
    NetJojo(u32 _inChannelCount, bool _outputScore) : outputScore{ _outputScore }
    {
        conv1 = register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 5).padding(2)));
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
        x = maxPool->forward(x);
        x = torch::relu(conv3->forward(x));
        // x = batch_norm->forward(x);
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
    string netName = "NetJojo";
    torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr, conv3 = nullptr;
    torch::nn::MaxPool2d maxPool = nullptr;
    torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    nn::BatchNorm2d batch_norm = nullptr;
};

std::string getDatasetPath(u32 _index)
{
    std::ostringstream path; path << "D:\\Prog\\blokusDataset\\dataset" << _index << ".bin";
    return path.str();
}

std::string getNetworkFilepath(string name)
{
    std::ostringstream path; path << "D:\\Prog\\blokusDataset\\" << name << ".pt";
    return path.str();
}

u32 findNumDatasetOnDisk()
{
    u32 index = 0;
    while (std::filesystem::exists(getDatasetPath(index)))
        index++;
    return index;
}

int main(int argc, char * argv[]) 
{
    cxxopts::Options options("Blockus AI", "A program to generate blockus games and use it as a dataset to train a network.");

    initBlokusAI();

    using namespace torch::indexing;

    u32 numDataset = findNumDatasetOnDisk();
    std::cout << "Num dataset : " << numDataset << std::endl;

    auto net = std::make_shared<NetJojo>(2, false);
    net->netName += "_endGame";
    if(std::filesystem::exists(getNetworkFilepath(net->netName)))
        torch::load(net, getNetworkFilepath(net->netName));

    for (u32 i = 0; i < numDataset; ++i)
    {
        Dataset dataset;
        dataset.read(getDatasetPath(i));

        auto batches = dataset.constructTensors(16, { 40, 64 });
        torch::optim::SGD optimizer(net->parameters(), 0.03);

        float averageLoss = 0;
        u32 batchIndex = 0;
        for (auto& [batch_data, batch_labels] : batches)
        {
            optimizer.zero_grad();
            torch::Tensor prediction = net->forward(batch_data);
            torch::Tensor loss = net->outputScore ? torch::mse_loss(prediction.flatten(), batch_labels) : torch::binary_cross_entropy(prediction, batch_labels);
            loss.backward();
            optimizer.step();

            averageLoss += loss.item<float>();

            constexpr u32 reportingInterval = 200;
            if (++batchIndex % reportingInterval == 0)
            {
                std::cout << "Dataset: " << i << " | Batch: " << batchIndex << "/" << batches.size()
                          << " | Loss: " << averageLoss / reportingInterval << std::endl;

                averageLoss = 0;
            }
        }

        // Serialize model periodically as a checkpoint.
        std::cout << "Save " << getNetworkFilepath(net->netName) << std::endl;
        torch::save(net, getNetworkFilepath(net->netName));
    }

    return 0;
}

//int main()
//{
//    initBlokusAI();
//    GameGenerator generator;
//    {
//        BaseAI::Parameters paramFullRandom;
//        paramFullRandom.heuristic = BoardHeuristic::RemainingTiles;
//        paramFullRandom.moveHeuristic = MoveHeuristic::TileCount;
//        paramFullRandom.maxMoveToLookAt = 128;
//        paramFullRandom.numTurnToForceBestMoveHeuristic = 0;
//        generator.addAI("FullRandom", new Dummy_AI(paramFullRandom));
//    }
//#if 1
//    {
//        BaseAI::Parameters paramRushCenterThenRandom;
//        paramRushCenterThenRandom.heuristic = BoardHeuristic::RemainingTiles;
//        paramRushCenterThenRandom.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
//        paramRushCenterThenRandom.maxMoveToLookAt = 128;
//        paramRushCenterThenRandom.numTurnToForceBestMoveHeuristic = 3;
//        generator.addAI("RushCenterThenRandom", new Dummy_AI(paramRushCenterThenRandom));
//    }
//    {
//        BaseAI::Parameters paramFullCenter;
//        paramFullCenter.heuristic = BoardHeuristic::RemainingTiles;
//        paramFullCenter.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
//        paramFullCenter.maxMoveToLookAt = 16;
//        paramFullCenter.numTurnToForceBestMoveHeuristic = 3;
//        generator.addAI("FullCenter", new Dummy_AI(paramFullCenter));
//    }
//    {
//        BaseAI::Parameters paramReachSpace;
//        paramReachSpace.heuristic = BoardHeuristic::RemainingTiles;
//        paramReachSpace.moveHeuristic = MoveHeuristic::WeightedReachableSpace;
//        paramReachSpace.maxMoveToLookAt = 16;
//        paramReachSpace.numTurnToForceBestMoveHeuristic = 3;
//        generator.addAI("ReachSpace", new Dummy_AI(paramReachSpace));
//    }
//    {
//        BaseAI::Parameters param;
//        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
//        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
//        param.maxMoveToLookAt = 32;
//        param.selectAmongNBestMoves = 8;
//        param.numTurnToForceBestMoveHeuristic = 3;
//        
//        param.maxDepth = 1;
//        generator.addAI("Alone_WReachSpace_Depth1", new AlonePlayer_AI(param));
//        param.maxDepth = 2;
//        generator.addAI("Alone_WReachSpace_Depth2", new AlonePlayer_AI(param));
//        
//        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted2;
//        param.maxDepth = 1;
//        generator.addAI("Alone_W2ReachSpace_Depth1", new AlonePlayer_AI(param));
//        param.maxDepth = 2;
//        generator.addAI("Alone_W2ReachSpace_Depth2", new AlonePlayer_AI(param));
//        
//        param.heuristic = BoardHeuristic::ReachableEmptySpace;
//        param.maxDepth = 1;
//        generator.addAI("Alone_ReachSpace_Depth1", new AlonePlayer_AI(param));
//        param.maxDepth = 2;
//        generator.addAI("Alone_ReachSpace_Depth2", new AlonePlayer_AI(param));
//        
//        param.selectAmongNBestMoves = 3;
//        param.maxMoveToLookAt = 32;
//        param.maxDepth = 3;
//        generator.addAI("Alone_WReachSpace_Depth3", new AlonePlayer_AI(param));
//    }
//    {
//        BaseAI::Parameters param;
//        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
//        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
//        param.maxMoveToLookAt = 32;
//        param.selectAmongNBestMoves = 4;
//        param.numTurnToForceBestMoveHeuristic = 3;
//        param.selectAmongNBestMoves = 4;
//        param.maxDepth = 3;
//        generator.addAI("Paranoid_TileCount_Depth3", new ParanoidFourPlayer_AI(param));
//
//        param.maxMoveToLookAt = 32;
//        param.maxDepth = 2;
//        generator.addAI("Paranoid_TileCount_Depth2", new ParanoidFourPlayer_AI(param));
//    }
//#endif
//
//    u32 startDatasetIndex = findNumDatasetOnDisk();
//    for (u32 epoch = 0; epoch < 50; ++epoch)
//    {
//        if (std::filesystem::exists(getDatasetPath(startDatasetIndex + epoch)))
//        {
//            std::cout << "For safety we dont overwrite existing dataset" << std::endl;
//            return -1;
//        }
//
//        u32 n = generator.numAIs();
//        const u32 numGames = 5000;
//        std::atomic<u32> numGameComplete = 0;
//        for (u32 i = 0; i < numGames; ++i)
//        {
//            s_threadPool.push_task([&]()
//                {
//                    generator.playGame(s_rand() % n, s_rand() % n, s_rand() % n, s_rand() % n);
//                    u32 num = numGameComplete.fetch_add(1);
//
//                    if (num % 100 == 0)
//                        std::cout << epoch << ": " << num << "/" << numGames << std::endl;
//                });
//        }
//        s_threadPool.wait_for_tasks();
//
//        generator.getDataset().serialize(getDatasetPath(startDatasetIndex + epoch));
//        generator.resetDataset();
//        generator.printResult();
//    }
//    
//	return 0;
//}

