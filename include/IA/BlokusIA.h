#pragma once

#include <bitset>

#include "BlokusGame.h"
#include "BlokusGameHelpers.h"

namespace BlokusIA
{
	extern PieceSymetries s_allPieces;
	void initBlokusIA();

	//-------------------------------------------------------------------------------------------------
	struct Move
	{
		Piece piece;
		u32 pieceIndex = u32(-1);
		ubyte2 position;

		bool isValid() const { return pieceIndex != u32(-1); }
	};

	//-------------------------------------------------------------------------------------------------
	class GameState
	{
	public:
		GameState();
		GameState play(const Move&) const;

		const Board& getBoard() const { return m_board; }
		u32 getPlayerTurn() const { return m_turn % 4; }
		u32 getTurnCount() const { return m_turn; }

		std::vector<Move> enumerateMoves(bool _sortByHeuristic) const;

		float computeHeuristic(const Move& _move) const;
		float computeBoardScore(Slot _player) const;

	private:
		Board m_board;
		std::bitset<BlokusGame::PiecesCount> m_remainingPieces[4];
		u32 m_turn = 0;
	};
}