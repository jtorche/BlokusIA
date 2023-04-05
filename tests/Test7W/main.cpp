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
	sevenWD::GameContext sevenWDContext(56774);
	sevenWD::GameState state(sevenWDContext);

	while (1)
	{
		state.printPlayablCards();
		bool pick=0;
		u32 cardIndex = 0;

		std::cout << "Pick(1) Or Burn(0) card :";
		std::cin >> pick;
		std::cout << "Cards? :";
		std::cin >> cardIndex;

		sevenWD::SpecialAction action = sevenWD::SpecialAction::Nothing;
		if (pick)
			action = state.pick(cardIndex);
		else
			state.burn(cardIndex);

		if (action != sevenWD::SpecialAction::Replay)
			state.nextPlayer();
	}

	std::cout << "Unit tests succeeded" << std::endl << std::endl;
	system("pause");

	return 0;
}
