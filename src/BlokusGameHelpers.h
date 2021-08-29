#pragma once

#include <iostream>
#include <array>
#include <vector>

#include "BlokusGame.h"


namespace BlokusIA
{
	struct Helpers
	{
		static std::vector<Piece> getAllPieces();
		static PieceSymetries getAllPieceSymetries();

		static uvec2 getRelativePosFromCorner(const Piece&, u32 _cornerIndex, ubyte2 _boardPos);
	};
}