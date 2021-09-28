#include "AI/IterativeAI.h"

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"

namespace blokusAI
{
    template<typename AI_t>
    void IterativeAI<AI_t>::startComputation(BoardHeuristic _heuristic, MoveHeuristic _moveHeuristic,  GameState _gameState)
    {
        DEBUG_ASSERT(m_thread == nullptr);

        m_bestMove = {};
        m_stopAI = false;

        m_thread = new std::thread([this, _heuristic, _moveHeuristic, _gameState]()
        {
            u32 maxDepth = 1;
            m_runningAI = new AI_t(maxDepth, _heuristic, _moveHeuristic);

            while (m_stopAI == false)
            {
                m_startClock = std::chrono::steady_clock::now();
                auto[move, score] = m_runningAI->findBestMove(_gameState);
                if (m_stopAI == false)
                {
                    std::lock_guard _{ m_mutex };

                    m_bestMove = { move, score, maxDepth };
                    m_nodePerSecond = m_runningAI->nodePerSecond();
                    delete m_runningAI;
                    m_runningAI = new AI_t(++maxDepth, _heuristic, _moveHeuristic);

                    // avoid infinite depth
                    if (maxDepth == 100)
                        break;
                }
            }
        });
    }

    template<typename AI_t>
    void IterativeAI<AI_t>::stopComputation()
    {
        DEBUG_ASSERT(m_thread);
        {
            std::lock_guard _{ m_mutex };
            m_runningAI->m_stopAI = true;
            m_stopAI = true;
        }

        m_thread->join();
        delete m_thread;
        delete m_runningAI;
        m_thread = nullptr;
        m_runningAI = nullptr;
    }

    template<typename AI_t>
    float IterativeAI<AI_t>::nodePerSecond()
    {
        std::lock_guard _{ m_mutex };
        if (m_runningAI)
        {
            std::chrono::duration<float> timeSinceStart = std::chrono::steady_clock::now() - m_startClock;
            return float(m_runningAI->getNumNodeExplored()) / timeSinceStart.count();
        }
        else
            return m_nodePerSecond;
    }

    template<typename AI_t>
    BestMove IterativeAI<AI_t>::getBestMove()
    {
        {
            std::lock_guard _{ m_mutex };
            return m_bestMove;
        }
    }

    template class IterativeAI<FourPlayerMaxN_AI>;
    template class IterativeAI<ParanoidFourPlayer_AI>;
}
