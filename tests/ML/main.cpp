#include <array>
#include <iostream>
#include <filesystem>
#include <windows.h>

#include "ML/GameGenerator.h"

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/Dummy_AI.h"
#include "AI/AlonePlayer_AI.h"

//struct Net : torch::nn::Module {
//    Net(int64_t N, int64_t M) {
//        W = register_parameter("W", torch::randn({ N, M }));
//        b = register_parameter("b", torch::randn(M));
//    }
//    torch::Tensor forward(torch::Tensor input) {
//        return torch::addmm(b, input, W);
//    }
//    torch::Tensor W, b;
//};

using namespace blokusAI;
using namespace torch;

struct NetJojo : torch::nn::Module 
{
    NetJojo()
    {
        conv1 = register_module("conv1", nn::Conv2d(nn::Conv2dOptions(1, 64, { 5,5 })));
        conv2 = register_module("conv2", nn::Conv2d(nn::Conv2dOptions(1, 128, { 3,3 })));
        //fc2 = register_module("fc2", torch::nn::Linear(64, 32));
        //fc3 = register_module("fc3", torch::nn::Linear(32, 10));
    }

    // Implement the Net's algorithm.
    torch::Tensor forward(torch::Tensor x) {
        // Use one of many tensor manipulation functions.
        //x = torch::relu(fc1->forward(x.reshape({ x.size(0), 784 })));
        //x = torch::dropout(x, /*p=*/0.5, /*train=*/is_training());
        //x = torch::relu(fc2->forward(x));
        //x = torch::log_softmax(fc3->forward(x), /*dim=*/1);
        return x;
    }

    torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr;
};

int main2() 
{
    
    using namespace torch::indexing;

    
    //torch::Tensor dat = torch::rand({ 11 });
    nn::Conv2dImpl net(nn::Conv2dOptions(2, 1, { 5,5 }));

    Dataset dataset;
    dataset.read("D:\\Prog\\blokusDataset\\dataset0.bin");

    auto tensors = dataset.constructTensors(64, { 0, 12 });
    std::cout << tensors.size() << " : " << tensors[0].sizes() << std::endl;
    auto slice = tensors[0].index({ Slice(0,1), Slice(0,2), Slice(), Slice() });
    std::cout << slice << std::endl;
    //torch::Tensor result = net.forward(slice);
    //std::cout << result << std::endl;
    
#if 0
    // Create a new Net.
    auto net = std::make_shared<Net>();

    // Create a multi-threaded data loader for the MNIST dataset.
    auto data_loader = torch::data::make_data_loader(
        torch::data::datasets::MNIST("./data").map(
            torch::data::transforms::Stack<>()),
        /*batch_size=*/64);

    // Instantiate an SGD optimization algorithm to update our Net's parameters.
    torch::optim::SGD optimizer(net->parameters(), /*lr=*/0.01);

    for (size_t epoch = 1; epoch <= 10; ++epoch) {
        size_t batch_index = 0;
        // Iterate the data loader to yield batches from the dataset.
        for (auto& batch : *data_loader) {
            // Reset gradients.
            optimizer.zero_grad();
            // Execute the model on the input data.
            torch::Tensor prediction = net->forward(batch.data);
            // Compute a loss value to judge the prediction of our model.
            torch::Tensor loss = torch::nll_loss(prediction, batch.target);
            // Compute gradients of the loss w.r.t. the parameters of our model.
            loss.backward();
            // Update the parameters based on the calculated gradients.
            optimizer.step();
            // Output the loss and checkpoint every 100 batches.
            if (++batch_index % 100 == 0) {
                std::cout << "Epoch: " << epoch << " | Batch: " << batch_index
                    << " | Loss: " << loss.item<float>() << std::endl;
                // Serialize your model periodically as a checkpoint.
                torch::save(net, "net.pt");
            }
        }
    }
#endif

    return 0;
}

int main()
{
    initBlokusAI();
    GameGenerator generator;
    {
        BaseAI::Parameters paramFullRandom;
        paramFullRandom.heuristic = BoardHeuristic::RemainingTiles;
        paramFullRandom.moveHeuristic = MoveHeuristic::TileCount;
        paramFullRandom.maxMoveToLookAt = 128;
        paramFullRandom.numTurnToForceBestMoveHeuristic = 0;
        generator.addAI("FullRandom", new Dummy_AI(paramFullRandom));
    }
#if 1
    {
        BaseAI::Parameters paramRushCenterThenRandom;
        paramRushCenterThenRandom.heuristic = BoardHeuristic::RemainingTiles;
        paramRushCenterThenRandom.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        paramRushCenterThenRandom.maxMoveToLookAt = 128;
        paramRushCenterThenRandom.numTurnToForceBestMoveHeuristic = 3;
        generator.addAI("RushCenterThenRandom", new Dummy_AI(paramRushCenterThenRandom));
    }
    {
        BaseAI::Parameters paramFullCenter;
        paramFullCenter.heuristic = BoardHeuristic::RemainingTiles;
        paramFullCenter.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        paramFullCenter.maxMoveToLookAt = 16;
        paramFullCenter.numTurnToForceBestMoveHeuristic = 3;
        generator.addAI("FullCenter", new Dummy_AI(paramFullCenter));
    }
    {
        BaseAI::Parameters paramReachSpace;
        paramReachSpace.heuristic = BoardHeuristic::RemainingTiles;
        paramReachSpace.moveHeuristic = MoveHeuristic::WeightedReachableSpace;
        paramReachSpace.maxMoveToLookAt = 16;
        paramReachSpace.numTurnToForceBestMoveHeuristic = 3;
        generator.addAI("ReachSpace", new Dummy_AI(paramReachSpace));
    }
    {
        BaseAI::Parameters param;
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        param.maxMoveToLookAt = 32;
        param.selectAmongNBestMoves = 8;
        param.numTurnToForceBestMoveHeuristic = 3;
        
        param.maxDepth = 1;
        generator.addAI("Alone_WReachSpace_Depth1", new AlonePlayer_AI(param));
        param.maxDepth = 2;
        generator.addAI("Alone_WReachSpace_Depth2", new AlonePlayer_AI(param));
        
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted2;
        param.maxDepth = 1;
        generator.addAI("Alone_W2ReachSpace_Depth1", new AlonePlayer_AI(param));
        param.maxDepth = 2;
        generator.addAI("Alone_W2ReachSpace_Depth2", new AlonePlayer_AI(param));
        
        param.heuristic = BoardHeuristic::ReachableEmptySpace;
        param.maxDepth = 1;
        generator.addAI("Alone_ReachSpace_Depth1", new AlonePlayer_AI(param));
        param.maxDepth = 2;
        generator.addAI("Alone_ReachSpace_Depth2", new AlonePlayer_AI(param));
        
        param.selectAmongNBestMoves = 3;
        param.maxMoveToLookAt = 32;
        param.maxDepth = 3;
        generator.addAI("Alone_WReachSpace_Depth3", new AlonePlayer_AI(param));
    }
    {
        BaseAI::Parameters param;
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        param.maxMoveToLookAt = 32;
        param.selectAmongNBestMoves = 4;
        param.numTurnToForceBestMoveHeuristic = 3;
        param.selectAmongNBestMoves = 4;
        param.maxDepth = 3;
        generator.addAI("Paranoid_TileCount_Depth3", new ParanoidFourPlayer_AI(param));

        param.maxMoveToLookAt = 32;
        param.maxDepth = 2;
        generator.addAI("Paranoid_TileCount_Depth2", new ParanoidFourPlayer_AI(param));
    }
#endif

    for (u32 epoch = 0; epoch < 100; ++epoch)
    {
        std::ostringstream outputPath; outputPath << "D:\\Prog\\blokusDataset\\dataset" << epoch << ".bin";
        if (std::filesystem::exists(outputPath.str()))
        {
            std::cout << "For safety we dont overwrite existing dataset" << std::endl;
            return -1;
        }

        u32 n = generator.numAIs();
        const u32 numGames = 5000;
        std::atomic<u32> numGameComplete = 0;
        for (u32 i = 0; i < numGames; ++i)
        {
            s_threadPool.push_task([&]()
            {
                generator.playGame(rand() % n, rand() % n, rand() % n, rand() % n);
                u32 num = numGameComplete.fetch_add(1);

                if (num % 100 == 0)
                    std::cout << epoch << ": " << num << "/" << numGames << std::endl;
            });
        }
        s_threadPool.wait_for_tasks();

        generator.getDataset().serialize(outputPath.str());
        generator.resetDataset();
        generator.printResult();
    }
    
    
    
	return 0;
}
