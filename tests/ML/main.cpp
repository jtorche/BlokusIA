#include <windows.h>

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/Dummy_AI.h"
#include "AI/AlonePlayer_AI.h"
#include "AI/IterativeAI.h"
#include "ML/HeuristicImpl.h"

using namespace blokusAI;

int main() 
{
    initBlokusAI();

    BaseAI::Parameters aiParam[4];

#if 0
    aiParam[0].moveHeuristic = MoveHeuristic::TileCount_DistCenter;
    aiParam[0].heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
    aiParam[0].maxMoveToLookAt = 16;
    aiParam[0].selectAmongNBestMoves = 1;
    IterativeAI<ParanoidFourPlayer_AI> AI1;
#endif
#if 1
    CustomHeuristicImpl customHeuristic(
        { { "D:/Prog/blokusDataset/model_0_16_simplecnn.pt", 16 },
          { "D:/Prog/blokusDataset/model_17_40_simplecnn.pt", 40 },
          { "D:/Prog/blokusDataset/model_41_84_simplecnn.pt", 84 } 
        }, BlokusNet::Model::Model_SimpleCnn, true
    );
    aiParam[0].maxDepth = 4;
    aiParam[0].moveHeuristic = MoveHeuristic::MultiSource;
    aiParam[0].heuristic = BoardHeuristic::Custom;
    aiParam[0].maxMoveToLookAt = 16;
    aiParam[0].maxMoveInRecursion = 16;
    aiParam[0].selectAmongNBestMoves = 1;
    aiParam[0].customHeuristic = &customHeuristic;
    aiParam[0].multiSourceParam.m_numPieceAtCenter = 12;
    aiParam[0].multiSourceParam.m_numPiecesWithBridge = 4;
    IterativeAI<FastParanoidFourPlayer_AI> AI1;
#endif

    aiParam[1].moveHeuristic = MoveHeuristic::TileCount_DistCenter;
    aiParam[1].heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
    aiParam[1].maxMoveToLookAt = 32;
    aiParam[1].maxMoveInRecursion = 32;
    aiParam[1].selectAmongNBestMoves = 1;
    IterativeAI<ParanoidFourPlayer_AI> AI2;

    aiParam[2].moveHeuristic = MoveHeuristic::TileCount_DistCenter;
    aiParam[2].heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
    aiParam[2].maxMoveToLookAt = 8;
    aiParam[2].maxMoveInRecursion = 8;
    aiParam[2].selectAmongNBestMoves = 1;
    IterativeAI<ParanoidFourPlayer_AI> AI3;

    aiParam[3].moveHeuristic = MoveHeuristic::TileCount_DistCenter;
    aiParam[3].heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
    aiParam[3].maxMoveToLookAt = 4;
    aiParam[3].maxMoveInRecursion = 4;
    aiParam[3].selectAmongNBestMoves = 1;
    IterativeAI<ParanoidFourPlayer_AI> AI4;

    for (auto& p : aiParam)
        p.monothread = false;

    GameState gameState;
    u32 turn = 0;

    auto playTurn = [&](auto& _ai, u32 _playerIndex) -> float
    {
        std::cout << "--------- Turn " << turn << " ------------\n";
        std::cout << "Board at begining of turn " << turn << ", Player " << _playerIndex+1 << " to play\n";
        gameState.getBoard().print();

        _ai.startComputation(aiParam[_playerIndex], gameState);
        do { Sleep(_playerIndex==0 ? 10000 : 1000); } while (_ai.getBestMove().depth == u32(-1));
        _ai.stopComputation();
        
        std::cout << "Move depth:" << _ai.getBestMove().depth << std::endl;
        std::cout << "Stats: " << _ai.nodePerSecond() << " node/sec, outcome: " << _ai.getBestMove().playerScore << std::endl;

        Move move = _ai.getBestMove().move;
        if (move.isValid())
            gameState = gameState.play(move);
        else
            gameState = gameState.skip();

        turn++;
        return _ai.getBestMove().playerScore;

    };

    while (1)
    {
        float scores[4];
        scores[0] = playTurn(AI1, turn%4);
        scores[1] = playTurn(AI2, turn%4);
        scores[2] = playTurn(AI3, turn%4);
        scores[3] = playTurn(AI4, turn%4);

        std::cout << std::endl;
        u32 noMoveForPlayer = 0;
        for (u32 i = 0; i < 4; ++i)
        {
            Slot p = Slot(u32(Slot::P0) + i);
            std::cout << "Played tiles for player " << i + 1 << " : " << gameState.getPlayedPieceTiles(p) << "(" << !gameState.noMoveLeft(p) << "), Score=" << scores[i] << std::endl;
            noMoveForPlayer += gameState.noMoveLeft(p);
        }

        system("pause");

        if (noMoveForPlayer == 4)
            break;
    }

    return 0;
}