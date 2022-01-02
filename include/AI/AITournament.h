#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class AITournament
    {
    public:
        struct AIInTournament
        {
            std::string m_aiName;
            std::unique_ptr<BaseAI> m_ai;
            u32 m_numMatchPlayed = 0;
            int m_score = 0;
        };

        AITournament() = default;
        void addAI(std::string _name, BaseAI* _ai);



    private:
        std::mutex m_mutex;
        std::vector<std::unique_ptr<AIInTournament>> m_allAIs;
    };
}
