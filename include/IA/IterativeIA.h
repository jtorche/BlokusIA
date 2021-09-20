#pragma once

#include "BlokusIA.h"
#include <chrono>

namespace BlokusIA
{
    //-------------------------------------------------------------------------------------------------
    template<typename IA_t>
    class IterativeIA
    {
    public:
        IterativeIA() = default;
        void startComputation(BoardHeuristic, GameState);
        void stopComputation();

        float nodePerSecond();
        std::pair<Move, u32> getBestMove();

    private:
        std::mutex m_mutex;
        Move m_bestMove;
        u32 m_bestMoveDepth = u32(-1);
        float m_nodePerSecond = 0;
        std::chrono::steady_clock::time_point m_startClock;

        std::thread * m_thread = nullptr;
        IA_t* m_runningIA = nullptr;
        std::atomic<bool> m_stopIA = false;
    };
}