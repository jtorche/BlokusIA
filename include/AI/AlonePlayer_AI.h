#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class AlonePlayer_AI : public BaseAI
    {
    public:
        using Score = float;
        AlonePlayer_AI(const BaseAI::Parameters& _parameters)
            : BaseAI(_parameters)
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState);

        Score computeScore(const GameState& _gameState)
        {
            m_numHeuristicEvaluated++;
            return _gameState.computeBoardScore(Slot(m_playerTurn + u32(Slot::P0)), m_params.heuristic);
        }

    private:
        Score evalPositionRec(const GameState& _gameState, u32 _depth);
        u32 m_playerTurn = 0;
    };
}