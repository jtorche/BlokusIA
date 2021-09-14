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
	FourPlayerMaxN_IA IA(3, BoardHeuristic::ReachableEmptySpace);
	u32 numTurn = 0;
	while(numTurn < 20)
	{
		Move move = IA.findBestMove(gameState);
		if (move.isValid())
			gameState = gameState.play(move);
		else break;

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