#include "BlokusIA.h"
#include "BlokusGameHelpers.h"

namespace BlokusIA
{
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
		newGameState.m_turn = (turn + 1) % 4;

		return newGameState;
	}

	//-------------------------------------------------------------------------------------------------
	std::vector<Move> GameState::enumerateMoves()
	{
		std::vector<Move> moves;
		Slot playerToMove = Slot(u32(Slot::P0) + getPlayerTurn());

		Board::PlayableSlots slots;
		u32 numSlots = m_board.computeValidSlotsForPlayer(playerToMove, slots);

		moves.reserve(m_remainingPieces[getPlayerTurn()].count() * numSlots);
		for (u32 piece = 0; piece < s_allPieces.size(); ++piece)
		{
			if (m_remainingPieces[getPlayerTurn()].test(piece))
			{
				u32 pieceIndex = 0;
				for (const Piece& p : s_allPieces[piece])
				{
					for (u32 i = 0; i < numSlots; ++i)
					{
						std::array<ubyte2, Piece::MaxPlayableCorners> pieceMoves;
						u32 numMoveForPiece = m_board.getPiecePlayablePositions(playerToMove, p, slots[i], pieceMoves, m_turn == 0xFFFFffff);

						for (u32 j = 0; j < numMoveForPiece; ++j)
						{
							moves.push_back({ p, pieceIndex, pieceMoves[j] });
						}
					}
					pieceIndex++;
				}
			}
		}

		return moves;
	}

	//-------------------------------------------------------------------------------------------------
	float GameState::computeHeuristic(const Move& _move) const
	{
		
	}
}