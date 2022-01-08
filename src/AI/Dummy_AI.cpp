#include "AI/Dummy_AI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    std::pair<Move, float> Dummy_AI::findBestMove(const GameState& _gameState)
    {
        m_playerTurn = _gameState.getPlayerTurn();

        start();
        auto moves = _gameState.enumerateMoves(m_params.moveHeuristic);
        if(moves.empty())
            moves = _gameState.enumerateMoves(MoveHeuristic::TileCount);

        _gameState.findCandidatMoves(m_params.maxMoveToLookAt, moves, m_params.numTurnToForceBestMoveHeuristic);

        if (moves.empty())
            return {};

        stop();

        u32 bestMoveIndex = u32(s_rand()) % moves.size();
        return moves[bestMoveIndex];
    }
}