#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class FourPlayerMaxN_AI : public BaseAI
    {
    public:
        using Score = std::array<float, 4>;
        FourPlayerMaxN_AI(u32 _maxDepth, BoardHeuristic _heuristic = BoardHeuristic::RemainingTiles, MoveHeuristic _moveHeuristic = MoveHeuristic::TileCount)
            : m_maxDepth{ _maxDepth } 
            , m_heuristic{ _heuristic }
            , m_moveHeuristic{ _moveHeuristic }
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState);

        Score computeScore(const GameState& _gameState)
        {
            m_numHeuristicEvaluated++;
            return { _gameState.computeBoardScore(Slot::P0, m_heuristic),
                     _gameState.computeBoardScore(Slot::P1, m_heuristic),
                     _gameState.computeBoardScore(Slot::P2, m_heuristic),
                     _gameState.computeBoardScore(Slot::P3, m_heuristic) };
        }

    private:
        Score evalPositionRec(const GameState& _gameState, u32 _depth);

        u32 m_maxDepth = 0;
        BoardHeuristic m_heuristic;
        MoveHeuristic m_moveHeuristic;
    };
}
