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

struct Net : torch::nn::Module {
    Net(int64_t N, int64_t M) {
        W = register_parameter("W", torch::randn({ N, M }));
        b = register_parameter("b", torch::randn(M));
    }
    torch::Tensor forward(torch::Tensor input) {
        return torch::addmm(b, input, W);
    }
    torch::Tensor W, b;
};

using namespace blokusAI;

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
