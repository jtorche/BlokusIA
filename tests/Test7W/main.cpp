#include <array>
#include <iostream>
#include <windows.h>

#include "AI/7WDuel/GameController.h"

namespace sevenWD
{
	void costTest();
}

int main()
{
	using namespace sevenWD;
	GameContext sevenWDContext(u32(time(nullptr)));
	GameController game(sevenWDContext);

	std::vector<Move> moves;
	Move move;
	u32 turn = 0;
	do
	{
		game.enumerateMoves(moves);
		move = moves[sevenWDContext.rand()() % moves.size()];

		const Card& card = game.m_gameState.getPlayableCard(move.playableCard);
		std::cout << "Turn " << turn++ << ", Player " << game.m_gameState.getCurrentPlayerTurn() << ", Action " << u32(move.action) << " with ";
		card.print();
		std::cout << std::endl;
		
	} 
	while (!game.play(move));

	std::cout << "Player " << (game.m_state == GameController::State::WinPlayer0 ? "0" : "1") << " has won a " << u32(game.m_winType) << " win.\n";
	system("pause");
	return 0;
}

#if 0
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
#endif
