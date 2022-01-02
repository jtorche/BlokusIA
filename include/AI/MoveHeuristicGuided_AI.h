#pragma once

#include "BlokusAI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    class MoveHeuristicGuided_AI : public BaseAI
    {
    public:
        using Score = float;
        MoveHeuristicGuided_AI(const BaseAI::Parameters& _parameters)
            : BaseAI{ _parameters }
        {}

        std::pair<Move, float> findBestMove(const GameState& _gameState) override
        {
            start();
            auto moves = _gameState.enumerateMoves(m_params.moveHeuristic);
            if (moves.empty())
                moves = _gameState.enumerateMoves(MoveHeuristic::TileCount);

            _gameState.findCandidatMoves(m_params.maxMoveToLookAt, moves);
            stop();

            if (moves.empty())
                return {};
            else
                return moves[rand() % moves.size()];
        }

        Score computeScore(const GameState&)
        {
            return 0;
        }
    };
}
