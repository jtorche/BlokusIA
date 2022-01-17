#include "AI/AlonePlayer_AI.h"

namespace blokusAI
{
    //-------------------------------------------------------------------------------------------------
    std::pair<Move, float> AlonePlayer_AI::findBestMove(const GameState& _gameState)
    {
        m_playerTurn = _gameState.getPlayerTurn();

        start();
        auto moves = _gameState.findMovesToLookAt(m_params.moveHeuristic, m_params.maxMoveToLookAt, m_params.numTurnToForceBestMoveHeuristic, &m_params.multiSourceParam, m_params.customHeuristic);
        
        if (moves.empty())
            return {};

        auto evalPosLambda = [&](const auto& move)
        {
            return evalPositionRec(_gameState.play(move.first), 1);
        };

        std::vector<std::future<float>> asyncScores(moves.size());
        std::transform(moves.begin(), moves.end(), asyncScores.begin(),
            [&](const auto& move) -> std::future<float>
        {
                if (m_params.monothread)
                {
                    std::promise<float> scorePromise;
                    scorePromise.set_value(evalPosLambda(move));
                    return scorePromise.get_future();
                }
                else
                    return s_threadPool.submit([&]() -> float { return evalPosLambda(move); });
        });

        std::vector<float> scores(asyncScores.size());
        std::transform(asyncScores.begin(), asyncScores.end(), scores.begin(),
            [&](std::future<float>& _score)
        {
            return _score.get();
        });

        u32 bestMoveIndex = GameState::getBestMoveIndex(scores, m_params.selectAmongNBestMoves);

        stop();

        return { moves[bestMoveIndex].first, scores[bestMoveIndex] };
    }

    //-------------------------------------------------------------------------------------------------
    AlonePlayer_AI::Score AlonePlayer_AI::evalPositionRec(const GameState& _gameState, u32 _depth)
    {
        m_numNodesExplored++;

        if (_depth >= m_params.maxDepth || m_stopAI)
            return computeScore(_gameState);

        GameState state = _gameState;
        while (m_playerTurn != state.getPlayerTurn())
        {
            auto moves = state.enumerateMoves(m_params.moveHeuristic);
            if (moves.empty())
                moves = state.enumerateMoves(MoveHeuristic::TileCount);

            if (moves.empty())
                state = state.skip();
            else
            {
                state.findCandidatMoves(1, moves, 0);
                state = state.play(moves[0].first);
            }
        }

        auto moves = state.findMovesToLookAt(m_params.moveHeuristic, m_params.maxMoveToLookAt, m_params.numTurnToForceBestMoveHeuristic, &m_params.multiSourceParam, m_params.customHeuristic);

        if (moves.empty())
        {
            return evalPositionRec(state.skip(), _depth + 1);
        }
        else
        {
            Score bestScore = -std::numeric_limits<float>::infinity(); 

            for (size_t i = 0; i < moves.size(); ++i)
            {
                Score score = evalPositionRec(state.play(moves[i].first), _depth + 1);
                if (score > bestScore)
                    bestScore = score;
            }

            return bestScore;
        }
    }
}