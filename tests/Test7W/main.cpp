#include <array>
#include <iostream>
#include <windows.h>

#include "AI/7WDuel/GameController.h"

namespace sevenWD
{
	void costTest();

	const char* toString(WinType _type)
	{
		switch (_type)
		{
		case WinType::Civil:
			return "Civil";
		case WinType::Military:
			return "Military";
		case WinType::Science:
			return "Science";
		default:
			return "None";
		}
	}

	const char* toString(Move::Action _action)
	{
		switch (_action)
		{
		case Move::Action::BuildWonder:
			return "BuildWonder";
		case Move::Action::Pick:
			return "Pick";
		case Move::Action::Burn:
			return "Burn";
		case Move::Action::ScienceToken:
			return "ScienceToken";
		default:
			return "None";
		}
	}


}

int main()
{
	using namespace sevenWD;
	GameContext sevenWDContext(u32(time(nullptr)));
	GameController game(sevenWDContext);

	std::vector<Move> moves;
	Move move;
	do
	{
		game.enumerateMoves(moves);

		if (game.m_gameState.getCurrentPlayerTurn() == 0)
		{
			constexpr std::array<u8, 4> actionScore{ { 2, 0, 1, 0 } };
			std::sort(moves.begin(), moves.end(), [&](const Move& _a, const Move& _b)
				{
					return actionScore[u32(_a.action)] > actionScore[u32(_b.action)];
				});
			move = moves[0];
		}
		else
			move = moves[sevenWDContext.rand()() % moves.size()];

		game.m_gameState.printGameState();
		std::cout << "Age " << game.m_gameState.getCurrentAge() + 1 << ", Player " << game.m_gameState.getCurrentPlayerTurn() + 1 << ": ";

		if (move.action == Move::Action::ScienceToken)
		{
			const Card& card = move.playableCard != u8(-1) ? game.m_gameState.getPlayableScienceToken(move.playableCard) : sevenWDContext.getCard(move.additionalId);
			std::cout << "Take science token "; card.print();
		}
		else if (move.action == Move::Action::BuildWonder)
		{
			const Card& card = game.m_gameState.getCurrentPlayerWonder(move.wonderIndex);
			std::cout << "Build wonder "; card.print(); 
		}
		else
		{
			const Card& card = game.m_gameState.getPlayableCard(move.playableCard);
			std::cout << toString(move.action) << " "; card.print();
		}

		std::cout << std::endl;
		
	} 
	while (!game.play(move));

	std::cout << "Player " << (game.m_state == GameController::State::WinPlayer0 ? "1" : "2") << " has won a " << toString(game.m_winType) << " win.\n";
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
