#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	class GenericMinMax_AI : public BaseAI
	{
	public:
        GenericMinMax_AI(const BaseAI::Parameters& _parameters)
            : BaseAI{ _parameters }
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState) override;

		float computeScore(Slot _maxPlayer, const GameState& _gameState)
		{
            m_numHeuristicEvaluated++;
            return Strategy::computeScore(_maxPlayer, m_params.heuristic, _gameState);
		}

	private:
		float evalPositionRec(Slot _maxPlayer, const GameState& _gameState, u32 _depth, vec2 _a_b);
	};
}
