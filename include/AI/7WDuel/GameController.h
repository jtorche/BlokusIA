#pragma once

#include "GameEngine.h"

namespace sevenWD
{
	struct Move
	{
		enum Action : u8 { Pick, Burn, BuildWonder };
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

		void enumerateMoves(std::vector<Move>&);
		bool play(Move _move);
	
		GameState m_gameState;
		WinType m_winType = WinType::None;

		enum class State
		{
			Play,
			PickScienceToken,
			WinPlayer0,
			WinPlayer1
		};
		State m_state = State::Play;
	};
}