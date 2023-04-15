#include "AI/7WDuel/GameController.h"

namespace sevenWD
{
	void GameController::enumerateMoves(std::vector<Move>& _moves)
	{
		_moves.clear();
		if (m_state == State::Play)
		{
			for (u8 i = 0; i < m_gameState.m_numPlayableCards; ++i)
			{
				const Card& card = m_gameState.getPlayableCard(i);
				u32 cost = m_gameState.getCurrentPlayerCity().computeCost(card, m_gameState.getOtherPlayerCity());
				if (cost <= m_gameState.getCurrentPlayerCity().m_gold)
					_moves.push_back(Move{ i, Move::Action::Pick });

				_moves.push_back(Move{ i, Move::Action::Burn });
			}

			for (u8 i = 0; i < m_gameState.getCurrentPlayerCity().m_unbuildWonderCount; ++i)
			{
				Wonders wonder = m_gameState.getCurrentPlayerCity().m_unbuildWonders[i];
				const Card& wonderCard = m_gameState.m_context.getWonder(wonder);

				u32 cost = m_gameState.getCurrentPlayerCity().computeCost(wonderCard, m_gameState.getOtherPlayerCity());
				if (cost <= m_gameState.getCurrentPlayerCity().m_gold)
				{
					for (u8 burnIndex = 0; burnIndex < m_gameState.m_numPlayableCards; ++burnIndex)
					{
						Move move{ burnIndex, Move::Action::BuildWonder, i };

						switch (wonder)
						{
						case Wonders::Zeus:
							break;
						case Wonders::CircusMaximus:
							break;
						case Wonders::GreatLibrary:
							break;
						case Wonders::Mausoleum:
							break;
						default:
							_moves.push_back(move);
							break;
						}
					}
				}
			}
		}
		else if (m_state == State::PickScienceToken)
		{
			for (u8 i = 0; i < m_gameState.m_numScienceToken; ++i)
			{
				Move move{ u8(-1), Move::Action::ScienceToken };
				move.playableCard = i;

				_moves.push_back(move);
			}
		}
		else
		{
			DEBUG_ASSERT(0);
		}
	}

	bool GameController::play(Move _move)
	{
		sevenWD::SpecialAction action = sevenWD::SpecialAction::Nothing;
		if (_move.action == Move::Pick)
			action = m_gameState.pick(_move.playableCard);
		else if (_move.action == Move::Burn)
			m_gameState.burn(_move.playableCard);
		else if (_move.action == Move::BuildWonder)
			action = m_gameState.buildWonder(_move.playableCard, _move.wonderIndex, _move.additionalId);
		else if (_move.action == Move::ScienceToken)
		{
			m_gameState.pickScienceToken(_move.playableCard);
			DEBUG_ASSERT(m_state == State::PickScienceToken);
		}

		if (action == sevenWD::SpecialAction::TakeScienceToken)
		{
			m_state = State::PickScienceToken;
			return false;
		}

		if (action == sevenWD::SpecialAction::MilitaryWin || action == sevenWD::SpecialAction::ScienceWin)
		{
			m_winType = action == sevenWD::SpecialAction::MilitaryWin ? WinType::Military : WinType::Science;
			m_state = m_gameState.getCurrentPlayerTurn() == 0 ? State::WinPlayer0 : State::WinPlayer1;
			return true;
		}

		sevenWD::GameState::NextAge ageState = m_gameState.nextAge();
		if (ageState == sevenWD::GameState::NextAge::None)
		{
			if (action != sevenWD::SpecialAction::Replay)
				m_gameState.nextPlayer();
		}
		else if (ageState == sevenWD::GameState::NextAge::EndGame)
		{
			m_winType = WinType::Civil;
			m_state = m_gameState.findWinner() == 0 ? State::WinPlayer0 : State::WinPlayer1;
			return true;
		}

		m_state = State::Play;
		return false;
	}
}