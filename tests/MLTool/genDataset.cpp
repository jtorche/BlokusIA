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

using namespace blokusAI;
using namespace torch;


std::string getDatasetPath(string _outputFolder, string _datasetBaseName, u32 _index)
{
    std::ostringstream path; path << _outputFolder << "\\" << _datasetBaseName  << _index << ".bin";
    return path.str();
}

u32 findNumDatasetOnDisk(string _outputFolder, string _datasetBaseName)
{
    u32 index = 0;
    while (std::filesystem::exists(getDatasetPath(_outputFolder, _datasetBaseName, index)))
        index++;
    return index;
}

int genDataset(string _outputFolder, string _datasetBaseName, u32 _numDataset, u32 _numGamePerDataset) 
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
        BaseAI::Parameters param;
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        param.maxMoveToLookAt = 32;
        param.maxMoveInRecursion = 32;
        param.numTurnToForceBestMoveHeuristic = 3;
        param.selectAmongNBestMoves = 3;
        param.maxDepth = 2;
        generator.addAI("Alone_WReachSpace_Depth2", new AlonePlayer_AI(param));
        
        param.selectAmongNBestMoves = 2;
        param.maxDepth = 3;
        generator.addAI("Alone_WReachSpace_Depth3", new AlonePlayer_AI(param));
    }
    {
        BaseAI::Parameters param;
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
        param.moveHeuristic = MoveHeuristic::MultiSource;
        param.selectAmongNBestMoves = 1;
        param.numTurnToForceBestMoveHeuristic = 3;
        param.maxDepth = 4;
        param.maxMoveToLookAt = 10;
        param.maxMoveInRecursion = 10;
        param.multiSourceParam.m_numPiecesWithBridge = 4;
        param.multiSourceParam.m_numPieceAtCenter = 6;
        generator.addAI("Paranoid_MultiSrc_Depth4", new ParanoidFourPlayer_AI(param));
    }
    {
        BaseAI::Parameters param;
        param.heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
        param.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
        param.maxMoveToLookAt = 32;
        param.maxMoveInRecursion = 32;
        param.numTurnToForceBestMoveHeuristic = 3;
        param.selectAmongNBestMoves = 2;

        param.maxDepth = 3;
        generator.addAI("Paranoid_TileCount_Depth3", new ParanoidFourPlayer_AI(param));
        param.maxDepth = 2;
        generator.addAI("Paranoid_TileCount_Depth2", new ParanoidFourPlayer_AI(param));
    }
#endif

    u32 startDatasetIndex = findNumDatasetOnDisk(_outputFolder, _datasetBaseName);
    for (u32 epoch = 0; epoch < _numDataset; ++epoch)
    {
        string datasetPath = getDatasetPath(_outputFolder, _datasetBaseName, startDatasetIndex + epoch);
        if (std::filesystem::exists(datasetPath))
        {
            std::cout << "For safety we dont overwrite existing dataset" << std::endl;
            return -1;
        }

        u32 n = generator.numAIs();
        const u32 numGames = _numGamePerDataset;
        std::atomic<u32> numGameComplete = 0;
        for (u32 i = 0; i < numGames; ++i)
        {
            s_threadPool.push_task([&]()
            {
                generator.playGame(s_rand() % n, s_rand() % n, s_rand() % n, s_rand() % n);
                u32 num = numGameComplete.fetch_add(1);

                if (num % 100 == 0)
                    std::cout << epoch << ": " << num << "/" << numGames << std::endl;
            });
        }
        s_threadPool.wait_for_tasks();

        generator.getDataset().serialize(datasetPath);
        generator.resetDataset();
        generator.printResult();
    }
    
	return 0;
}

