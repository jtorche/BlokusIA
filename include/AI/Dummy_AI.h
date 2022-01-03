#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class Dummy_AI : public BaseAI
    {
    public:
        using Score = float;
        Dummy_AI(const BaseAI::Parameters& _parameters)
            : BaseAI(_parameters)
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState);

        Score computeScore(const GameState& _gameState)
        {
            m_numHeuristicEvaluated++;
            return _gameState.computeBoardScore(Slot(m_playerTurn + u32(Slot::P0)), m_params.heuristic);
        }

    private:
        u32 m_playerTurn = 0;
    };
}