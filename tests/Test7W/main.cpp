#include <array>
#include <iostream>
#include <windows.h>

#include "AI/7WDuel/GameEngine.h"

namespace sevenWD
{
	void costTest();
}

int main()
{
	sevenWD::costTest();

	std::cout << "Sizoef Card = " << sizeof(sevenWD::Card) << std::endl;
	std::cout << "Sizoef GameState = " << sizeof(sevenWD::GameState) << std::endl;
	sevenWD::GameContext sevenWDContext;
	sevenWD::GameState state(sevenWDContext);

	std::cout << "Unit tests succeeded" << std::endl << std::endl;
	system("pause");

	return 0;
}
