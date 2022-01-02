#include <array>
#include <iostream>

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/MoveHeuristicGuided_AI.h"
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
    BaseAI::Parameters parameters = 
    {
        1, 16,
        BoardHeuristic::ReachableEmptySpaceWeighted,
        MoveHeuristic::TileCount,
        false
    };

    IterativeAI<AlonePlayer_AI> AI;
    u32 numTurn = 0;
    while (1)
    {
        AI.startComputation(parameters, gameState);

        u32 thinking = 1;
        while (thinking)
        {
            std::cout << "\nContinue ? ";
            std::cin >> thinking;

            std::cout << "Cur depth:" << AI.getBestMove().depth << std::endl;
            std::cout << "Stats: " << AI.nodePerSecond() << " node/sec, curMoveScore: " << AI.getBestMove().playerScore << std::endl;
        }

        AI.stopComputation();
        getGlobalCache().resetStats();

        Move move = AI.getBestMove().move;
        if (move.isValid())
            gameState = gameState.play(move);
        else
        {
            gameState = gameState.skip();
            std::cout << "!! Player 1 has lost." << std::endl;
            break;
        }

        for (u32 i = 1; i < 4; ++i)
        {
            auto moves = gameState.enumerateMoves(MoveHeuristic::TileCount);
            gameState.findCandidatMoves(1, moves);
            if (moves.empty())
            {
                gameState = gameState.skip();
                std::cout << "!! Player " << i + 1 << " has lost." << std::endl;
            }
            else
                gameState = gameState.play(moves[0].first);
        }

		numTurn++;

		gameState.getBoard().print();

        for (Slot s : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
            std::cout << u32(s) << "(" << gameState.getPlayedPieceTiles(s) << ") :" << gameState.computeBoardScore(s, parameters.heuristic) << std::endl;
	
        system("pause");
    }
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}
