#pragma once

#include "BlokusGame.h"
#include <bitset>

namespace BlokusIA
{
	extern PieceSymetries s_allPieces;
	void initBlokusIA();

	struct Move
	{
		Piece piece;
		u32 pieceIndex;
		ubyte2 position;
	};

	class GameState
	{
	public:
		GameState();
		GameState play(const Move&) const;

		const Board& getBoard() const { return m_board; }
		u32 getPlayerTurn() const { return m_turn % 4; }

		std::vector<Move> enumerateMoves(bool _sortByHeuristic);

		float computeHeuristic(const Move& _move) const;

	private:
		Board m_board;
		std::bitset<20> m_remainingPieces[4];
		u32 m_turn = 0;
	};
}