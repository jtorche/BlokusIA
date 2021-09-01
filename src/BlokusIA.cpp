#include "BlokusIA.h"
#include "BlokusGameHelpers.h"

namespace BlokusIA
{
	PieceSymetries s_allPieces = {};

	//-------------------------------------------------------------------------------------------------
	void initBlokusIA()
	{
		s_allPieces = Helpers::getAllPieceSymetries();
	}

	//-------------------------------------------------------------------------------------------------
	GameState::GameState()
	{
		// initially all pieces are playable
		for (auto& remaining : m_remainingPieces)
			remaining.set();
	}

	//-------------------------------------------------------------------------------------------------
	GameState GameState::play(const Move& _move) const
	{
		GameState newGameState = *this;

		u32 turn = getPlayerTurn();
		TIM_ASSERT(m_remainingPieces[turn].test(_move.pieceIndex));
		newGameState.m_remainingPieces[turn].reset(_move.pieceIndex);

		Slot player = Slot(u32(Slot::P0) + turn);
		TIM_ASSERT(m_board.canAddPiece(player, _move.piece, { _move.position.x, _move.position.y }));
		newGameState.m_board.addPiece(player, _move.piece, _move.position);
		newGameState.m_turn = turn + 1;

		return newGameState;
	}

	//-------------------------------------------------------------------------------------------------
	std::vector<Move> GameState::enumerateMoves(bool _sortByHeuristic)
	{
		std::vector<Move> moves;
		Slot playerToMove = Slot(u32(Slot::P0) + getPlayerTurn());

		Board::PlayableSlots slots;
		u32 numSlots = m_board.computeValidSlotsForPlayer(playerToMove, slots);

		moves.reserve(m_remainingPieces[getPlayerTurn()].count() * numSlots);

		// reverse order to look at big pieces first (to improve sorting later)
		for (auto it = s_allPieces.rbegin() ;  it != s_allPieces.rend() ; ++it)
		{
			u32 piece = (u32)std::distance(s_allPieces.begin(), it.base()) - 1;
			if (m_remainingPieces[getPlayerTurn()].test(piece))
			{
				for (const Piece& p : *it)
				{
					for (u32 i = 0; i < numSlots; ++i)
					{
						std::array<ubyte2, Piece::MaxPlayableCorners> pieceMoves;
						u32 numMoveForPiece = m_board.getPiecePlayablePositions(playerToMove, p, slots[i], pieceMoves, m_turn < 4);

						for (u32 j = 0; j < numMoveForPiece; ++j)
						{
							moves.push_back({ p, piece, pieceMoves[j] });
						}
					}
				}
			}
		}

		if (_sortByHeuristic)
		{
			std::sort(std::begin(moves), std::end(moves), [this](const Move& m1, const Move& m2)
			{
				return computeHeuristic(m1) > computeHeuristic(m2);
			});
		}
		return moves;
	}

	//-------------------------------------------------------------------------------------------------
	float GameState::computeHeuristic(const Move& _move) const
	{
		float closestDistToCenter = std::numeric_limits<float>::max();
		vec2 boardCenter = { Board::BoardSize / 2, Board::BoardSize / 2 };
		for (u32 i = 0; i < _move.piece.getNumTiles(); ++i)
		{
			vec2 tilePos = vec2{ 0.5f + (float)Piece::getTileX(_move.piece.getTile(i)), 0.5f + (float)Piece::getTileY(_move.piece.getTile(i)) };
			tilePos += vec2(_move.position.x, _move.position.y);
			closestDistToCenter = std::min(closestDistToCenter, linalg::length2(boardCenter - tilePos) / (2 * Board::BoardSize * Board::BoardSize));
		}

		return _move.piece.getNumTiles() * 10.0f +
			   _move.piece.getNumCorners() +
			   1.f - closestDistToCenter;
	}
}