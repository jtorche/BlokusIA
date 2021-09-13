#include <array>
#include <iostream>

#include "Core/Common.h"
#include "Core/type.h"

#include "IA/FourPlayerMaxN_IA.h"

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
        system("pause");
	}
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}