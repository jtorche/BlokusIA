#pragma once

#include <iostream>
#include <array>
#include <vector>

#include "BlokusGame.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	// For each piece, all possibles symetries to play the piece
	using PieceSymetries = std::array<tim::flat_hash_set<Piece>, BlokusGame::PiecesCount>;

	struct Helpers
	{
		static std::array<Piece, BlokusGame::PiecesCount> getAllPieces();
		static PieceSymetries getAllPieceSymetries();
	};
}