#include "IA/IterativeIA.h"

#include "IA/FourPlayerMaxN_IA.h"
#include "IA/ParanoidFourPlayer_IA.h"

namespace BlokusIA
{
    template<typename IA_t>
    void IterativeIA<IA_t>::startComputation(BoardHeuristic _heuristic, GameState _gameState)
    {
        DEBUG_ASSERT(m_thread == nullptr);

        m_bestMove = {};
        m_bestMoveDepth = {};
        m_stopIA = false;

        m_thread = new std::thread([this, _heuristic, _gameState]()
        {
            u32 maxDepth = 1;
            m_runningIA = new IA_t(maxDepth, _heuristic);

            while (m_stopIA == false)
            {
                Move move = m_runningIA->findBestMove(_gameState);
                if (m_stopIA == false)
                {
                    std::lock_guard _{ m_mutex };
                    m_bestMove = move;
                    m_bestMoveDepth = maxDepth;
                    m_nodePerSecond = m_runningIA->nodePerSecond();
                    delete m_runningIA;
                    m_runningIA = new IA_t(++maxDepth, _heuristic);
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
        return m_nodePerSecond;
    }

    template<typename IA_t>
    std::pair<Move, u32> IterativeIA<IA_t>::getBestMove()
    {
        {
            std::lock_guard _{ m_mutex };
            return { m_bestMove, m_bestMoveDepth };
        }
    }

    template class IterativeIA<FourPlayerMaxN_IA>;
    template class IterativeIA<ParanoidFourPlayer_IA>;
}