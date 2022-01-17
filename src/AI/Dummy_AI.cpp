#include "AI/Dummy_AI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    std::pair<Move, float> Dummy_AI::findBestMove(const GameState& _gameState)
    {
        m_playerTurn = _gameState.getPlayerTurn();

        start();
        auto moves = _gameState.findMovesToLookAt(m_params.moveHeuristic, m_params.maxMoveToLookAt, m_params.numTurnToForceBestMoveHeuristic, &m_params.multiSourceParam, m_params.customHeuristic);

        if (moves.empty())
            return {};

        std::stable_sort(std::begin(moves), std::end(moves), [](const auto& m1, const auto& m2) { return m1.second > m2.second; });

        if(moves.size() > m_params.selectAmongNBestMoves)
            moves.resize(m_params.selectAmongNBestMoves);

        stop();

        u32 bestMoveIndex = u32(s_rand()) % moves.size();
        return moves[bestMoveIndex];
    }
}