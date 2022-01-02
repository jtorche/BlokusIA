#pragma once

#include "AI/AITournament.h"

namespace blokusAI
{
    void AITournament::addAI(std::string _name, BaseAI* _ai)
    {
        auto ai = std::make_unique<AIInTournament>();
        ai->m_ai = std::unique_ptr<BaseAI>(_ai);
        ai->m_aiName = _name;
        m_allAIs.push_back(std::move(ai));
    }

}
