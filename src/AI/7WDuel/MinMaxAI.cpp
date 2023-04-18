#include "AI/7WDuel/MinMaxAI.h"
#include <future>

#include "Core/thread_pool.h"

namespace sevenWD
{
#ifdef _DEBUG
    thread_pool s_threadPool(1);
#else
    thread_pool s_threadPool;
#endif

    MinMaxAI::MinMaxAI(u32 _maxDepth, bool _monothread) : m_maxDepth{ _maxDepth }, m_monothread{ _monothread }
    {

    }

	//-------------------------------------------------------------------------------------------------
    std::pair<Move, float> MinMaxAI::findBestMove(const GameController& _gameState)
	{
        std::vector<Move> moves;
        _gameState.enumerateMoves(moves);

        m_numMoves += moves.size();
        m_numNodeExplored++;

        std::vector<std::future<float>> asyncScores(moves.size());
        std::atomic<float> b = std::numeric_limits<float>::max();

        auto evalPosLambda = [&](const Move& _move)
        {
            GameController newGameState = _gameState;
            newGameState.play(_move);

            float score = evalRec(_gameState.m_gameState.getCurrentPlayerTurn(), newGameState, 1, { -std::numeric_limits<float>::max(), b });
            b.store(std::min(b.load(), score));
            return score;
        };

        std::transform(moves.begin(), moves.end(), asyncScores.begin(),
            [&](const Move& move) -> std::future<float>
            {
                if (m_monothread)
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

        auto bestIt = std::max_element(scores.begin(), scores.end());
        u32 bestScoreIndex = (u32)std::distance(scores.begin(), bestIt);
        return std::make_pair(moves[bestScoreIndex], scores[bestScoreIndex]);
	}

	//-------------------------------------------------------------------------------------------------
	float MinMaxAI::evalRec(u32 _maxPlayer, const GameController& _gameState, u32 _depth, vec2 _a_b)
	{
        bool isMaxPlayerTurn = _gameState.m_gameState.getCurrentPlayerTurn() == _maxPlayer;

        if (_depth >= m_maxDepth || m_stopAI)
        {
            n_numLeafEpxlored++;
            return computeScore(_gameState, _maxPlayer);
        }

        std::vector<Move> moves;
        _gameState.enumerateMoves(moves);

        m_numMoves += moves.size();
        m_numNodeExplored++;

        // The "maxPlayer" supposes the opponent minimize his score, reverse for the "minPlayer"
        float score = isMaxPlayerTurn ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

        auto minmax = [isMaxPlayerTurn](float s1, float s2)
        {
            return isMaxPlayerTurn ? std::min(s1, s2) : std::max(s1, s2);
        };

        for (size_t i = 0; i < moves.size(); ++i)
        {
            GameController newGameState = _gameState;
            newGameState.play(moves[i]);

            score = minmax(evalRec(_maxPlayer, newGameState, _depth + 1, _a_b), score);
            if (isMaxPlayerTurn)
            {
                if (score <= _a_b.x)
                    return score;
                _a_b.y = std::min(_a_b.y, score);
            }
            else
            {
                if (score >= _a_b.y)
                    return score;
                _a_b.x = std::max(_a_b.x, score);
            }
        }

        return score;
	}

    //-------------------------------------------------------------------------------------------------
    float MinMaxAI::computeScore(const GameController& _gameState, u32 _maxPlayer) const
    {
        const PlayerCity& player = _gameState.m_gameState.getPlayerCity(_maxPlayer);
        const PlayerCity& opponent = _gameState.m_gameState.getPlayerCity((_maxPlayer + 1) % 2);

        return float(player.computeVictoryPoint(opponent)) - float(opponent.computeVictoryPoint(player));
    }
}
