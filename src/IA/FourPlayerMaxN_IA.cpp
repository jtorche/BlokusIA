#include "IA/FourPlayerMaxN_IA.h"

namespace BlokusIA
{
    //-------------------------------------------------------------------------------------------------
    Move FourPlayerMaxN_IA::findBestMove(const GameState& _gameState)
    {
        start();
        auto moves = _gameState.enumerateMoves();
        _gameState.findCandidatMoves(m_moveHeuristic, maxMoveToLookAt(_gameState), moves);

        if (moves.empty())
            return {};

        std::vector<std::future<float>> asyncScores(moves.size());
        std::transform(moves.begin(), moves.end(), asyncScores.begin(),
            [&](const Move& move) -> std::future<float>
        {
            return s_threadPool.submit([&]() -> float { return evalPositionRec(_gameState.play(move), 1)[_gameState.getPlayerTurn()]; });
        });

        std::vector<float> scores(asyncScores.size());
        std::transform(asyncScores.begin(), asyncScores.end(), scores.begin(),
            [&](std::future<float>& _score)
        {
            return _score.get();
        });

        u32 bestMoveIndex = GameState::getBestMoveIndex(scores);

        stop();
        return moves[bestMoveIndex];
    }

    //-------------------------------------------------------------------------------------------------
    FourPlayerMaxN_IA::Score FourPlayerMaxN_IA::evalPositionRec(const GameState& _gameState, u32 _depth)
    {
        m_numNodesExplored++;

        if (_depth >= m_maxDepth || m_stopIA)
            return computeScore(_gameState);

        auto moves = _gameState.enumerateMoves();
        _gameState.findCandidatMoves(m_moveHeuristic, maxMoveToLookAt(_gameState), moves);

        if (moves.empty())
        {
            return evalPositionRec(_gameState.skip(), _depth + 1);
        }
        else
        {
            Score bestScore = {};
            u32 bestScoreIndex = 0;
            for (size_t i = 0; i < moves.size(); ++i)
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
}