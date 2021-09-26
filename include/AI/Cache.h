#pragma once

#include "Core/flat_hash_map.h"
#include "AI/BlokusIA.h"

namespace BlokusIA
{
    class GameStateCache
    {
        std::mutex m_mutex;
        ska::flat_hash_map<GameState, float> m_boardScore[u32(BoardHeuristic::BoardHeuristic_Count)][4];
        u32 m_cacheMiss = 0;
        u32 m_cacheHit = 0;

    public:
        GameStateCache() = default;
        float computeBoardScore(const GameState&, Slot _player, BoardHeuristic);
        void resetStats();
        void reset();

        float getCacheHitRatio() const;
    };

    inline float GameStateCache::computeBoardScore(const GameState& _gameState, Slot _player, BoardHeuristic _heuristic)
    {
        return _gameState.computeBoardScoreInner(_player, _heuristic);
    }
}
