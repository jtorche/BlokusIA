#include <array>
#include <iostream>

#include "AI/FourPlayerMaxN_IA.h"
#include "AI/ParanoidFourPlayer_IA.h"
#include "AI/IterativeIA.h"
#include "AI/Cache.h"

void runTest();

using namespace BlokusIA;

int main()
{
	initBlokusIA();
    printAllPieces();

	// run some unit test
	runTest();

	GameState gameState;
    //const BoardHeuristic heuristic = BoardHeuristic::ReachableEmptySpaceOnly;
    const BoardHeuristic heuristic = BoardHeuristic::RemainingTiles;
    const MoveHeuristic moveHeuristic = MoveHeuristic::ExtendingReachableSpace;

    IterativeIA<FourPlayerMaxN_IA> IA;
    u32 numTurn = 0;
    while (1)
    {
        IA.startComputation(heuristic, moveHeuristic, gameState);

        u32 thinking = 1;
        while (thinking)
        {
            std::cout << "\nContinue ? ";
            std::cin >> thinking;

            std::cout << "Cur depth:" << IA.getBestMove().depth << std::endl;
            std::cout << "Stats: " << IA.nodePerSecond() << " node/sec, curMoveScore: " << IA.getBestMove().playerScore << std::endl;
        }

        IA.stopComputation();
        getGlobalCache().resetStats();

        Move move = IA.getBestMove().move;
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
            std::cout << u32(s) << "(" << gameState.getPlayedPieceTiles(s) << ") :" << gameState.computeBoardScore(s, heuristic) << std::endl;
	
        system("pause");
    }
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}