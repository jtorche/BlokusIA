#include <iostream>
#include <array>
#include "timCore/type.h"
#include "timCore/Common.h"

#include "BlokusIA.h"
#include "BlokusGameHelpers.h"

void runTest();

using namespace BlokusIA;

int main()
{
	initBlokusIA();

	// run some unit test
	runTest();

	GameState gameState;
	for (u32 turn = 0; turn < 12; ++turn)
	{
		auto moves = gameState.enumerateMoves(true);
		gameState = gameState.play(moves[0]);
	}
	
	gameState.getBoard().print();
	system("pause");

	return 0;
}