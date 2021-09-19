#include <array>
#include <iostream>

#include "IA/FourPlayerMaxN_IA.h"
#include "IA/ParanoidFourPlayer_IA.h"

void runTest();

using namespace BlokusIA;

int main()
{
	initBlokusIA();

	// run some unit test
	runTest();

	GameState gameState;
    const BoardHeuristic heuristic = BoardHeuristic::RemainingTiles;

    ParanoidFourPlayer_IA IA(4, heuristic);
    u32 numTurn = 0;
    while (numTurn < 20)
    {
        Move move = IA.findBestMove(gameState);
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
            auto moves = gameState.enumerateMoves(true);
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
        std::cout << "Num H evaluated: " << IA.m_numHeuristicEvaluated << ", "<< IA.nodePerSecond() << " node/sec" << std::endl;
        //for (Slot s : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
        //{
        //    float upperBound = gameState.computeScoreUpperBound(s, heuristic);
        //    std::cout << u32(s) << ":" << gameState.computeScoreLowerBound(s, heuristic) << " < "
        //        << gameState.computeBoardScore(s, heuristic) << " < "
        //        << upperBound << std::endl;
        //}

        system("pause");
	}
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}