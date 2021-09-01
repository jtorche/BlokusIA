#pragma once

#include "BlokusGame.h"
#include <bitset>

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
		std::bitset<21> m_remainingPieces[4];
		u32 m_turn = 0;
	};

	//-------------------------------------------------------------------------------------------------
	class TwoPlayerMinMaxIA
	{
	public:
		TwoPlayerMinMaxIA(u32 _maxDepth) : m_maxDepth{ _maxDepth } {}

		Move findBestMove(const GameState& _gameState);

		static float computeScore(bool _isP0P2_MaxPlayer, const GameState& _gameState)
		{
			return (_isP0P2_MaxPlayer ? 1 : -1) * _gameState.computeBoardScore(Slot::P0) + _gameState.computeBoardScore(Slot::P2) - _gameState.computeBoardScore(Slot::P1) - _gameState.computeBoardScore(Slot::P3);
		}

		size_t maxMoveToLookAt(const GameState& _gameState) const;

	private:
		float evalPositionRec(bool _isMaxPlayerTurn, const GameState& _gameState, u32 _depth, vec2 _a_b);

		u32 m_maxDepth = 0;
	};
}