#pragma once

#include "BlokusIA.h"

namespace BlokusIA
{
    //-------------------------------------------------------------------------------------------------
    class FourPlayerMaxN_IA
    {
    public:
        using Score = std::array<float, 4>;
        FourPlayerMaxN_IA(u32 _maxDepth) : m_maxDepth{ _maxDepth } {}

        Move findBestMove(const GameState& _gameState);

        static Score computeScore(const GameState& _gameState)
        {
            return { _gameState.computeBoardScore(Slot::P0),
                     _gameState.computeBoardScore(Slot::P1),
                     _gameState.computeBoardScore(Slot::P2),
                     _gameState.computeBoardScore(Slot::P3) };
        }

        size_t maxMoveToLookAt(const GameState& _gameState) const;

    private:
        Score evalPositionRec(const GameState& _gameState, u32 _depth);

        u32 m_maxDepth = 0;
    };
}