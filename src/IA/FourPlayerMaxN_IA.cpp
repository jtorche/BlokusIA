#include "IA/FourPlayerMaxN_IA.h"

namespace BlokusIA
{
    //-------------------------------------------------------------------------------------------------
    Move FourPlayerMaxN_IA::findBestMove(const GameState& _gameState)
    {
        auto moves = _gameState.enumerateMoves(true);
        moves.resize(std::min(moves.size(), maxMoveToLookAt(_gameState)));

        if (moves.empty())
            return {};

        std::vector<float> scores(moves.size());
        std::transform(moves.begin(), moves.end(), scores.begin(),
            [&](const Move& move) -> float
        {
            return evalPositionRec(_gameState.play(move), 0)[_gameState.getPlayerTurn()];
        });

        auto best = std::max_element(scores.begin(), scores.end());
        return moves[std::distance(scores.begin(), best)];
    }

    //-------------------------------------------------------------------------------------------------
    size_t FourPlayerMaxN_IA::maxMoveToLookAt(const GameState& _gameState) const
    {
        return 16;
    }

    //-------------------------------------------------------------------------------------------------
    FourPlayerMaxN_IA::Score FourPlayerMaxN_IA::evalPositionRec(const GameState& _gameState, u32 _depth)
    {
        if (_depth >= m_maxDepth)
            return computeScore(_gameState);

        auto moves = _gameState.enumerateMoves(true);

        Score bestScore = {};
        u32 bestScoreIndex = 0;
        for (size_t i = 0; i < std::min(moves.size(), maxMoveToLookAt(_gameState)); ++i)
        {
            Score score = evalPositionRec(_gameState.play(moves[i]), _depth + 1);
            // Each player try to maximize its own score, regardless of the other players
            if (score[_gameState.getPlayerTurn()] > bestScore[_gameState.getPlayerTurn()])
            {
                bestScore = score;
                bestScoreIndex = u32(i);
            }
        }

        return bestScore;
    }
}