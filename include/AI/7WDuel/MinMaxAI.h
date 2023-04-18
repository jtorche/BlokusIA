#pragma once

#include "GameController.h"

namespace sevenWD
{
	//-------------------------------------------------------------------------------------------------
	class MinMaxAI
	{
	public:
		MinMaxAI(u32 _maxDepth = 10, bool _monothread = false);

        std::pair<Move, float> findBestMove(const GameController& _gameState);

		double getAvgMovesPerTurn() const { return double(m_numMoves.load()) / m_numNodeExplored.load(); }

	private:
		float evalRec(u32 _maxPlayer, const GameController& _gameState, u32 _depth, vec2 _a_b);
		float computeScore(const GameController& _gameState, u32 _maxPlayer) const;

	private:
		u32 m_maxDepth = 10;
		bool m_monothread = false;
		std::atomic<u64> m_numNodeExplored = 0, m_numMoves = 0, n_numLeafEpxlored = 0;
		std::atomic<bool> m_stopAI;
	};
}
