#pragma once

#include "GameEngine.h"

namespace sevenWD
{
	struct Move
	{
		enum Action : u8 { Pick, Burn, BuildWonder, ScienceToken, Count };
		u8 playableCard;
		Action action;
		u8 wonderIndex = u8(-1);
		u8 additionalId = u8(-1);
	};

	enum class WinType
	{
		None,
		Civil,
		Military,
		Science
	};

	struct GameController
	{
		GameController(const GameContext& _context) : m_gameState(_context) {}

		void enumerateMoves(std::vector<Move>&) const;
		bool play(Move _move);

		bool filterMove(Move _move) const;
	
		GameState m_gameState;
		WinType m_winType = WinType::None;

		enum class State
		{
			Play,
			PickScienceToken,
			GreatLibraryToken,
			GreatLibraryTokenThenReplay,
			WinPlayer0,
			WinPlayer1
		};
		State m_state = State::Play;
	};
}