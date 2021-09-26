#pragma once

#include <chrono>

#include "BlokusAI.h"

namespace BlokusIA
{
    //-------------------------------------------------------------------------------------------------
    struct BestMove
    {
        Move move;
        float playerScore = 0;
        u32 depth = u32(-1);
    };

    //-------------------------------------------------------------------------------------------------
    template<typename IA_t>
    class IterativeIA
    {
    public:
        IterativeIA() = default;
        void startComputation(BoardHeuristic, MoveHeuristic, GameState);
        void stopComputation();

        float nodePerSecond();
        BestMove getBestMove();

    private:
        std::mutex m_mutex;
        BestMove m_bestMove;
        float m_nodePerSecond = 0;
        std::chrono::steady_clock::time_point m_startClock;

        std::thread * m_thread = nullptr;
        IA_t* m_runningIA = nullptr;
        std::atomic<bool> m_stopIA = false;
    };
}