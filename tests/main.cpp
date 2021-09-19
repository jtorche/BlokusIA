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
    const BoardHeuristic heuristic = BoardHeuristic::ReachableEmptySpace;

    FourPlayerMaxN_IA IA(3, heuristic);
    u32 numTurn = 0;
    while (numTurn < 20)
    {
        Move move = IA.findBestMove(gameState);
        if (move.isValid())
            gameState = gameState.play(move);
        else
        {
            gameState = gameState.skip();
            std::cout << "!! Player 0 has lost." << std::endl;
            break;
        }

        for (u32 i = 0; i < 3; ++i)
        {
            auto moves = gameState.enumerateMoves(true);
            if (moves.empty())
            {
                gameState = gameState.skip();
                std::cout << "!! Player " << i + 2 << " has lost." << std::endl;
            }
            else
                gameState = gameState.play(moves[0]);
        }

		numTurn++;

		gameState.getBoard().print();
        std::cout << "Num H evaluated: " << IA.m_numHeuristicEvaluated << ", "<< IA.nodePerSecond() << " node/sec" << std::endl;
        for (Slot s : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
        {
            float upperBound = gameState.computeScoreUpperBound(s, heuristic);
            std::cout << u32(s) << ":" << gameState.computeScoreLowerBound(s, heuristic) << " < "
                << gameState.computeBoardScore(s, heuristic) << " < "
                << upperBound << std::endl;
        }

        system("pause");
	}
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}