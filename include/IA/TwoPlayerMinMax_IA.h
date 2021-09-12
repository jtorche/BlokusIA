#pragma once

#include "BlokusIA.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	class TwoPlayerMinMax_IA
	{
	public:
		TwoPlayerMinMax_IA(u32 _maxDepth, BoardHeuristic _heuristic = BoardHeuristic::RemainingTiles)
            : m_maxDepth{ _maxDepth }
            , m_heuristic{ _heuristic }
        {}

		Move findBestMove(const GameState& _gameState);

		float computeScore(bool _isP0P2_MaxPlayer, const GameState& _gameState)
		{
			return (_isP0P2_MaxPlayer ? 1 : -1) * (_gameState.computeBoardScore(Slot::P0, m_heuristic) + 
                                                   _gameState.computeBoardScore(Slot::P2, m_heuristic) - 
                                                   _gameState.computeBoardScore(Slot::P1, m_heuristic) - 
                                                   _gameState.computeBoardScore(Slot::P3, m_heuristic));
		}

		size_t maxMoveToLookAt(const GameState& _gameState) const;

	private:
		float evalPositionRec(bool _isMaxPlayerTurn, const GameState& _gameState, u32 _depth, vec2 _a_b);

		u32 m_maxDepth = 0;
        BoardHeuristic m_heuristic;
	};
}