#include "IA/TwoPlayerMinMax_IA.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	size_t TwoPlayerMinMax_IA::maxMoveToLookAt(const GameState& _gameState) const
	{
		if (_gameState.getTurnCount() < 16)
			return 8;
		else
			return 16;
	}

	//-------------------------------------------------------------------------------------------------
	Move TwoPlayerMinMax_IA::findBestMove(const GameState& _gameState)
	{
		auto moves = _gameState.enumerateMoves(true);
		moves.resize(std::min(moves.size(), maxMoveToLookAt(_gameState)));

		if (moves.empty())
			return {};

		std::vector<float> scores(moves.size());
		float b = std::numeric_limits<float>::max();
		std::transform(moves.begin(), moves.end(), scores.begin(),
			[&](const Move& move) -> float
			{
				float score = evalPositionRec(false, _gameState.play(move), 0, { -std::numeric_limits<float>::max(), b });
				b = std::min(b, score);
				return score;
			});
		
		auto best = std::max_element(scores.begin(), scores.end());
		return moves[std::distance(scores.begin(), best)];
	}

	//-------------------------------------------------------------------------------------------------
	float TwoPlayerMinMax_IA::evalPositionRec(bool _isMaxPlayerTurn, const GameState& _gameState, u32 _depth, vec2 _a_b)
	{
		bool isP0P2_MaxPlayer = _isMaxPlayerTurn == (_gameState.getPlayerTurn() % 2 == 0);

		if (_depth >= m_maxDepth)
			return computeScore(isP0P2_MaxPlayer, _gameState);

		auto moves = _gameState.enumerateMoves(true);

		// The "maxPlayer" supposes the opponent minimize his score, reverse for the "minPlayer"
		float score = _isMaxPlayerTurn ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

		auto minmax = [_isMaxPlayerTurn](float s1, float s2)
		{
			return _isMaxPlayerTurn ? std::min(s1, s2) : std::max(s1, s2);
		};

		for (size_t i = 0; i < std::min(moves.size(), maxMoveToLookAt(_gameState)); ++i)
		{
			score = minmax(evalPositionRec(!_isMaxPlayerTurn, _gameState.play(moves[i]), _depth + 1, _a_b), score);
			if (_isMaxPlayerTurn)
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