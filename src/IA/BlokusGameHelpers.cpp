#include "BlokusGameHelpers.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	std::array<Piece, BlokusGame::PiecesCount> Helpers::getAllPieces()
	{
		return
		{
			// 1
			// x
			Piece { Piece::build(0,0) },

			// 2
			// xx
			{ Piece::build(0,0), Piece::build(1,0) },

			// 3
			// xx
			// x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(0,1) },

			// 4
			// xxx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0) },

			// 5
			// xx
			// xx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(0,1), Piece::build(1,1) },

			// 6
			// xxx
			//  x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,1), Piece::build(1,1) },

			// 7
			// xxxx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0) },

			// 8
			// xxx
			// x  
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1) },

			// 9
			// xx
			//  xx  
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(1,2) },

			// 10
			// xxxxx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(4,0) },

			// 11
			// xxxx
			// x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(0,1) },

			// 12
			// xxx
			// x
			// x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1), Piece::build(0,2) },

			// 13
			// xxx
			//   xx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(2,1), Piece::build(3,1) },

			// 14
			// x
			// xxx
			//   x
			{ Piece::build(0,0), Piece::build(0,1), Piece::build(1,1), Piece::build(2,1), Piece::build(2,2) },

			// 15
			// x
			// xx
			// xx
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(1,0), Piece::build(1,1) },

			// 16
			// xx
			//  xx
			//   x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(2,1), Piece::build(2,2) },

			// 17
			// xxx
			// x x 
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1), Piece::build(2,1) },

			// 18
			//  x 
			// xxx
			//  x
			{ Piece::build(1,0), Piece::build(0,1), Piece::build(1,1), Piece::build(2,1), Piece::build(1,2) },

			// 19
			// xxxx
			//  x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(1,1) },

			// 20
			// xx
			//  xx
			//  x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(2,1), Piece::build(1,2) },

			// 21
			// xxx
			//  x
			//  x
			{ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(1,1), Piece::build(1,2) }
		};
	}

	//-------------------------------------------------------------------------------------------------
	PieceSymetries Helpers::getAllPieceSymetries()
	{
		std::array<Piece, BlokusGame::PiecesCount> pieces = getAllPieces();
		PieceSymetries symetries;

		u32 i = 0;
		for (const Piece& p : pieces)
		{
			for (bool flip : { false, true })
			{
				for (Rotation rot : { Rotation::Rot_0, Rotation::Rot_90, Rotation::Rot_180, Rotation::Rot_270 })
				{
					Piece rotatedP = p.rotate(flip ? Rotation::Flip_X : Rotation::Rot_0).rotate(rot);
					symetries[i].insert(rotatedP);
				}
			}

		    ++i;
		}

		return symetries;
	}
}