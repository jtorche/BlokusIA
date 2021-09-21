#include <array>
#include <iostream>

#include "IA/FourPlayerMaxN_IA.h"
#include "IA/ParanoidFourPlayer_IA.h"
#include "IA/IterativeIA.h"
#include "IA/Cache.h"

void runTest();

using namespace BlokusIA;

int main()
{
	initBlokusIA();

	// run some unit test
	runTest();

	GameState gameState;
    const BoardHeuristic heuristic = BoardHeuristic::ReachableEmptySpaceOnly;
    // const BoardHeuristic heuristic = BoardHeuristic::RemainingTiles;
    const MoveHeuristic moveHeuristic = MoveHeuristic::TileCount;

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

            std::cout << "Cur depth:" << IA.getBestMove().second << std::endl;
            std::cout << "Stats: " << IA.nodePerSecond() << " node/sec, cacheHitRatio: " << getGlobalCache().getCacheHitRatio() << std::endl;
        }

        IA.stopComputation();
        getGlobalCache().resetStats();

        Move move = IA.getBestMove().first;
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
            auto moves = gameState.enumerateMoves();
            gameState.findCandidatMoves(MoveHeuristic::TileCount, 1, moves);
            if (moves.empty())
            {
                gameState = gameState.skip();
                std::cout << "!! Player " << i + 1 << " has lost." << std::endl;
            }
            else
                gameState = gameState.play(moves[0]);
        }

		numTurn++;

		gameState.getBoard().print();

        for (Slot s : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
            std::cout << u32(s) << "(" << gameState.getPlayedPieceTiles(s) << ") :" << gameState.computeBoardScore(s, heuristic) << std::endl;
	}
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}