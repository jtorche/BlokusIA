#include "AI/IterativeAI.h"

#include "AI/FourPlayerMaxN_AI.h"
#include "AI/ParanoidFourPlayer_AI.h"
#include "AI/TwoPlayerMinMax_AI.h"
#include "AI/Dummy_AI.h"
#include "AI/AlonePlayer_AI.h"

namespace blokusAI
{
    template<typename AI_t>
    void IterativeAI<AI_t>::startComputation(const BaseAI::Parameters& _parameters,  GameState _gameState)
    {
        DEBUG_ASSERT(m_thread == nullptr);

        m_bestMove = {};
        m_stopAI = false;

        m_thread = new std::thread([this, parameter = _parameters, _gameState]() mutable
        {
            u32& maxDepth = parameter.maxDepth;
            maxDepth = 1;
            m_runningAI = new AI_t(parameter);

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
                    ++maxDepth;
                    m_runningAI = new AI_t(parameter);

                    // avoid infinite depth
                    if (maxDepth == 64)
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
    template class IterativeAI<FastParanoidFourPlayer_AI>;
    template class IterativeAI<TwoPlayerMinMax_AI>;
    template class IterativeAI<Dummy_AI>;
    template class IterativeAI<AlonePlayer_AI>;
}
