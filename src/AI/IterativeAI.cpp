#include "AI/IterativeAI.h"

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"

namespace BlokusIA
{
    template<typename IA_t>
    void IterativeIA<IA_t>::startComputation(BoardHeuristic _heuristic, MoveHeuristic _moveHeuristic,  GameState _gameState)
    {
        DEBUG_ASSERT(m_thread == nullptr);

        m_bestMove = {};
        m_stopIA = false;

        m_thread = new std::thread([this, _heuristic, _moveHeuristic, _gameState]()
        {
            u32 maxDepth = 1;
            m_runningIA = new IA_t(maxDepth, _heuristic, _moveHeuristic);

            while (m_stopIA == false)
            {
                m_startClock = std::chrono::steady_clock::now();
                auto[move, score] = m_runningIA->findBestMove(_gameState);
                if (m_stopIA == false)
                {
                    std::lock_guard _{ m_mutex };

                    m_bestMove = { move, score, maxDepth };
                    m_nodePerSecond = m_runningIA->nodePerSecond();
                    delete m_runningIA;
                    m_runningIA = new IA_t(++maxDepth, _heuristic, _moveHeuristic);

                    // avoid infinite depth
                    if (maxDepth == 100)
                        break;
                }
            }
        });
    }

    template<typename IA_t>
    void IterativeIA<IA_t>::stopComputation()
    {
        DEBUG_ASSERT(m_thread);
        {
            std::lock_guard _{ m_mutex };
            m_runningIA->m_stopIA = true;
            m_stopIA = true;
        }

        m_thread->join();
        delete m_thread;
        delete m_runningIA;
        m_thread = nullptr;
        m_runningIA = nullptr;
    }

    template<typename IA_t>
    float IterativeIA<IA_t>::nodePerSecond()
    {
        std::lock_guard _{ m_mutex };
        if (m_runningIA)
        {
            std::chrono::duration<float> timeSinceStart = std::chrono::steady_clock::now() - m_startClock;
            return float(m_runningIA->getNumNodeExplored()) / timeSinceStart.count();
        }
        else
            return m_nodePerSecond;
    }

    template<typename IA_t>
    BestMove IterativeIA<IA_t>::getBestMove()
    {
        {
            std::lock_guard _{ m_mutex };
            return m_bestMove;
        }
    }

    template class IterativeIA<FourPlayerMaxN_IA>;
    template class IterativeIA<ParanoidFourPlayer_IA>;
}