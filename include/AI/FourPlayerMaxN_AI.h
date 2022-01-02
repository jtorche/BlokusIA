#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class FourPlayerMaxN_AI : public BaseAI
    {
    public:
        using Score = std::array<float, 4>;
        FourPlayerMaxN_AI(const BaseAI::Parameters& _parameters)
            : BaseAI{ _parameters }
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState) override;

        Score computeScore(const GameState& _gameState)
        {
            m_numHeuristicEvaluated++;
            return { _gameState.computeBoardScore(Slot::P0, m_params.heuristic),
                     _gameState.computeBoardScore(Slot::P1, m_params.heuristic),
                     _gameState.computeBoardScore(Slot::P2, m_params.heuristic),
                     _gameState.computeBoardScore(Slot::P3, m_params.heuristic) };
        }

    private:
        Score evalPositionRec(const GameState& _gameState, u32 _depth);
    };
}
