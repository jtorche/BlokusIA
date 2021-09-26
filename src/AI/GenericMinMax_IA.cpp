#include "IA/GenericMinMax_IA.h"
#include "IA/TwoPlayerMinMax_IA.h"
#include "IA/ParanoidFourPlayer_IA.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
    std::pair<Move, float> GenericMinMax_IA<Strategy>::findBestMove(const GameState& _gameState)
	{
        start();

        auto moves = _gameState.enumerateMoves(m_moveHeuristic);
        if (moves.empty())
            moves = _gameState.enumerateMoves(MoveHeuristic::TileCount);

        _gameState.findCandidatMoves(maxMoveToLookAt(_gameState), moves);

		if (moves.empty())
			return {};

        std::vector<std::future<float>> asyncScores(moves.size());
        std::atomic<float> b = std::numeric_limits<float>::max();

        std::transform(moves.begin(), moves.end(), asyncScores.begin(),
            [&](const auto& move) -> std::future<float>
        {
            return s_threadPool.submit([&]() -> float 
            { 
                float score = evalPositionRec(Slot(_gameState.getPlayerTurn() + u32(Slot::P0)), _gameState.play(move.first), 1, { -std::numeric_limits<float>::max(), b });
                b.store(std::min(b.load(), score));
                return score;
            });
        });

        std::vector<float> scores(asyncScores.size());
        std::transform(asyncScores.begin(), asyncScores.end(), scores.begin(),
            [&](std::future<float>& _score)
        {
            return _score.get();
        });
		
        u32 bestMoveIndex = GameState::getBestMoveIndex(scores);

        stop();
        return { moves[bestMoveIndex].first, scores[bestMoveIndex] };
	}

	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	float GenericMinMax_IA<Strategy>::evalPositionRec(Slot _maxPlayer, const GameState& _gameState, u32 _depth, vec2 _a_b)
	{
        m_numNodesExplored++;

        bool isMaxPlayerTurn = Strategy::isMaxPlayerTurn(_maxPlayer, _gameState);

		if (_depth >= m_maxDepth || m_stopIA)
			return computeScore(_maxPlayer, _gameState);

        auto moves = _gameState.enumerateMoves(m_moveHeuristic);
        if (moves.empty())
            moves = _gameState.enumerateMoves(MoveHeuristic::TileCount);

        _gameState.findCandidatMoves(maxMoveToLookAt(_gameState), moves);

        if (moves.empty())
        {
            return evalPositionRec(_maxPlayer, _gameState.skip(), _depth + 1, _a_b);
        }
        else
        {
            // The "maxPlayer" supposes the opponent minimize his score, reverse for the "minPlayer"
            float score = isMaxPlayerTurn ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

            auto minmax = [isMaxPlayerTurn](float s1, float s2)
            {
                return isMaxPlayerTurn ? std::min(s1, s2) : std::max(s1, s2);
            };

            for (size_t i = 0; i < moves.size(); ++i)
            {
                score = minmax(evalPositionRec(_maxPlayer, _gameState.play(moves[i].first), _depth + 1, _a_b), score);
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
	}

    template class GenericMinMax_IA<TwoPlayerMinMaxStrategy>;
    template class GenericMinMax_IA<ParanoidStrategy>;
}