#include <array>
#include <iostream>
#include <windows.h>

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/Dummy_AI.h"
#include "AI/AlonePlayer_AI.h"
#include "AI/IterativeAI.h"
#include "AI/Cache.h"

void runTest();

using namespace blokusAI;

int main()
{
    initBlokusAI();
    printAllPieces();

	// run some unit test
	runTest();

	GameState gameState;
    BaseAI::Parameters parameters;
    parameters.moveHeuristic = MoveHeuristic::TileCount_DistCenter;
    parameters.heuristic = BoardHeuristic::RemainingTiles;
    parameters.maxMoveToLookAt = 16;
    parameters.selectAmongNBestMoves = 4;
    
    IterativeAI<ParanoidFourPlayer_AI> AI;
    u32 numTurn = 0;

    while (1)
    {
        AI.startComputation(parameters, gameState);

        //u32 thinking = 1;
        //while (thinking)
        //{
        //    std::cout << "\nContinue ? ";
        //    std::cin >> thinking;
        //    
        //
        //    std::cout << "Cur depth:" << AI.getBestMove().depth << std::endl;
        //    std::cout << "Stats: " << AI.nodePerSecond() << " node/sec, curMoveScore: " << AI.getBestMove().playerScore << std::endl;
        //}

        while(AI.getBestMove().depth == u32(-1))
            Sleep(50);

        AI.stopComputation();

        std::cout << "----------------------------------\n";
        std::cout << "Move depth:" << AI.getBestMove().depth << std::endl;
        std::cout << "Stats: " << AI.nodePerSecond() << " node/sec, outcome: " << AI.getBestMove().playerScore << std::endl;

        getGlobalCache().resetStats();

        Move move = AI.getBestMove().move;
        if (move.isValid())
            gameState = gameState.play(move);
        else
            gameState = gameState.skip();

        for (u32 i = 1; i < 4; ++i)
        {
            auto moves = gameState.enumerateMoves(MoveHeuristic::TileCount_DistCenter);
            gameState.findCandidatMoves(8, moves, 3);
            if (moves.empty())
                gameState = gameState.skip();
            else
                gameState = gameState.play(moves[u32(s_rand()) % moves.size()].first);
        }

		numTurn++;
        std::cout << "NumTurn " << numTurn*4 << "\n";
		gameState.getBoard().print();

        u32 noMoveForPlayer = 0;
        for (u32 i = 0; i < 4; ++i)
        {
            Slot p = Slot(u32(Slot::P0) + i);
            std::cout << "Played tiles for player " << i + 1 << " : " << gameState.getPlayedPieceTiles(p) << "(" << !gameState.noMoveLeft(p) << ")" << std::endl;
            noMoveForPlayer += gameState.noMoveLeft(p);
        }

        if (noMoveForPlayer == 4)
            break;
    }
	
	std::cout << "NumTurn " << numTurn*4 << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}
