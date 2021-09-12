#include "IA/GenericMinMax_IA.h"
#include "IA/TwoPlayerMinMax_IA.h"
#include "IA/ParanoidFourPlayer_IA.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	size_t GenericMinMax_IA<Strategy>::maxMoveToLookAt(const GameState& _gameState) const
	{
		if (_gameState.getTurnCount() < 16)
			return 8;
		else
			return 16;
	}

	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	Move GenericMinMax_IA<Strategy>::findBestMove(const GameState& _gameState)
	{
        start();

		auto moves = _gameState.enumerateMoves(true);
		moves.resize(std::min(moves.size(), maxMoveToLookAt(_gameState)));

		if (moves.empty())
			return {};

		std::vector<float> scores(moves.size());
		float b = std::numeric_limits<float>::max();
		std::transform(moves.begin(), moves.end(), scores.begin(),
			[&](const Move& move) -> float
			{
				float score = evalPositionRec(Slot(_gameState.getPlayerTurn() + u32(Slot::P0)), _gameState.play(move), 0, { -std::numeric_limits<float>::max(), b });
				b = std::min(b, score);
				return score;
			});
		
		auto best = std::max_element(scores.begin(), scores.end());

        stop();
		return moves[std::distance(scores.begin(), best)];
	}

	//-------------------------------------------------------------------------------------------------
    template<typename Strategy>
	float GenericMinMax_IA<Strategy>::evalPositionRec(Slot _maxPlayer, const GameState& _gameState, u32 _depth, vec2 _a_b)
	{
        m_numNodesExplored++;

        bool isMaxPlayerTurn = Strategy::isMaxPlayerTurn(_maxPlayer, _gameState);

		if (_depth >= m_maxDepth)
			return computeScore(_maxPlayer, _gameState);

		auto moves = _gameState.enumerateMoves(true);

		// The "maxPlayer" supposes the opponent minimize his score, reverse for the "minPlayer"
		float score = isMaxPlayerTurn ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

		auto minmax = [isMaxPlayerTurn](float s1, float s2)
		{
			return isMaxPlayerTurn ? std::min(s1, s2) : std::max(s1, s2);
		};

		for (size_t i = 0; i < std::min(moves.size(), maxMoveToLookAt(_gameState)); ++i)
		{
			score = minmax(evalPositionRec(_maxPlayer, _gameState.play(moves[i]), _depth + 1, _a_b), score);
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

    template class GenericMinMax_IA<TwoPlayerMinMaxStrategy>;
    template class GenericMinMax_IA<ParanoidStrategy>;
}