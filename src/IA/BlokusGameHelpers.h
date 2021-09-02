#pragma once

#include <array>
#include <iostream>
#include <vector>

#include "BlokusGame.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	// For each piece, all possibles symetries to play the piece
	using PieceSymetries = std::array<core::flat_hash_set<Piece>, BlokusGame::PiecesCount>;

	struct Helpers
	{
		static std::array<Piece, BlokusGame::PiecesCount> getAllPieces();
		static PieceSymetries getAllPieceSymetries();
	};
}