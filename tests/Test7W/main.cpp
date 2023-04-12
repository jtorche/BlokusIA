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
	sevenWD::GameContext sevenWDContext(u32(time(nullptr)));
	sevenWD::GameState state(sevenWDContext);

	u32 turn = 0;
	while (1)
	{
		std::cout << "Turn " << ++turn << std::endl;
		state.printPlayablCards();
		u32 pick=0;
		u32 cardIndex = 0;

		std::cout << "Pick(1) Or Burn(2) card :";
		std::cin >> pick;
		std::cout << "Cards? :";
		std::cin >> cardIndex;

		sevenWD::SpecialAction action = sevenWD::SpecialAction::Nothing;
		if (pick == 1)
			action = state.pick(cardIndex-1);
		else
			state.burn(cardIndex-1);

		if (action == sevenWD::SpecialAction::TakeScienceToken)
		{
			state.printAvailableTokens();

			u32 tokenIndex = 0;
			std::cout << "Pick science token:";
			std::cin >> tokenIndex;
			action = state.pickScienceToken(tokenIndex-1);
		}

		if (action == sevenWD::SpecialAction::MilitaryWin || action == sevenWD::SpecialAction::ScienceWin)
		{
			std::cout << "Winner is " << state.getCurrentPlayerTurn() << " (military or science win)" << std::endl;
			return 0;
		}

		sevenWD::GameState::NextAge ageState = state.nextAge();
		if (ageState == sevenWD::GameState::NextAge::None)
		{
			if (action != sevenWD::SpecialAction::Replay)
				state.nextPlayer();
		}
		else if (ageState == sevenWD::GameState::NextAge::EndGame)
		{
			std::cout << "Winner is " << state.findWinner() << std::endl;
			return 0;
		}
	}

	std::cout << "Unit tests succeeded" << std::endl << std::endl;
	system("pause");

	return 0;
}
