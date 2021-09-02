#include <iostream>
#include <array>

#include "Core/type.h"
#include "Core/Common.h"

#include "IA/BlokusIA.h"
#include "IA/BlokusGameHelpers.h"

void runTest();

using namespace BlokusIA;

int main()
{
	initBlokusIA();

	// run some unit test
	runTest();

	GameState gameState;
	TwoPlayerMinMaxIA IA(5);
	u32 numTurn = 0;
	while(numTurn < 20)
	{
		Move move = IA.findBestMove(gameState);
		if (move.isValid())
			gameState = gameState.play(move);
		else break;

		numTurn++;

		gameState.getBoard().print();
		system("pause");
	}
	
	std::cout << "NumTurn " << numTurn << "\n";
	gameState.getBoard().print();
	system("pause");

	return 0;
}