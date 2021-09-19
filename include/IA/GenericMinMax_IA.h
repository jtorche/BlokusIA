#pragma once

#include "BlokusIA.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	class GenericMinMax_IA : public BaseIA
	{
	public:
        GenericMinMax_IA(u32 _maxDepth, BoardHeuristic _heuristic = BoardHeuristic::RemainingTiles)
            : m_maxDepth{ _maxDepth }
            , m_heuristic{ _heuristic }
        {}

		Move findBestMove(const GameState& _gameState);

		float computeScore(Slot _maxPlayer, const GameState& _gameState)
		{
            m_numHeuristicEvaluated++;
            return Strategy::computeScore(_maxPlayer, m_heuristic, _gameState);
		}

	private:
		float evalPositionRec(Slot _maxPlayer, const GameState& _gameState, u32 _depth, vec2 _a_b);

		u32 m_maxDepth = 0;
        BoardHeuristic m_heuristic;
	};
}