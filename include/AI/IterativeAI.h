#pragma once

#include <chrono>

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    struct BestMove
    {
        Move move;
        float playerScore = 0;
        u32 depth = u32(-1);
    };

    //-------------------------------------------------------------------------------------------------
    template<typename AI_t>
    class IterativeAI
    {
    public:
        IterativeAI() = default;
        void startComputation(const BaseAI::Parameters&, GameState);
        void stopComputation();

        float nodePerSecond();
        BestMove getBestMove();

    private:
        std::mutex m_mutex;
        BestMove m_bestMove;
        float m_nodePerSecond = 0;
        std::chrono::steady_clock::time_point m_startClock;

        std::thread * m_thread = nullptr;
        AI_t* m_runningAI = nullptr;
        std::atomic<bool> m_stopAI = false;
    };
}
