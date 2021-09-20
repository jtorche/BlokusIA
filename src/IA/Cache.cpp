#include "IA/Cache.h"
#include "IA/BlokusIA.h"

namespace BlokusIA
{
    float GameStateCache::computeBoardScore(const GameState& _gameState, Slot _player, BoardHeuristic _heuristic)
    {
#if 0
        std::lock_guard _{ m_mutex };

        auto& cache = m_boardScore[u32(_heuristic)][u32(_player) - u32(Slot::P0)];
        auto it = cache.find(_gameState);
        if (it != cache.end())
        {
            m_cacheHit++;
            return it->second;
        }
        else
        {
            float score = _gameState.computeBoardScoreInner(_player, _heuristic);
            cache[_gameState] = score;
            m_cacheMiss++;
            return score;
        } 
#endif
        return _gameState.computeBoardScoreInner(_player, _heuristic);
    }

    void GameStateCache::resetStats()
    {
        m_cacheHit = 0;
        m_cacheMiss = 0;
    }

    void GameStateCache::reset()
    {
        std::lock_guard _{ m_mutex };

        resetStats();
        for (u32 h = 0; h < u32(BoardHeuristic::BoardHeuristic_Count); ++h)
        {
            for (u32 p = 0; p < 4; ++p)
                m_boardScore[h][p] = {};
        }
    }

    float GameStateCache::getCacheHitRatio() const
    {
        return float(m_cacheHit) / (m_cacheHit + m_cacheMiss);
    }
}
