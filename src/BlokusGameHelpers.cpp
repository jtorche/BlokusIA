#include "BlokusGameHelpers.h"

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	std::vector<Piece> Helpers::getAllPieces()
	{
		std::vector<Piece> pieces;

		// 1
		// x
		pieces.push_back({ Piece::build(0,0) });

		// 2
		// xx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0) });

		// 3
		// xx
		// x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(0,1) });

		// 4
		// xxx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0) });

		// 5
		// xx
		// xx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(0,1), Piece::build(1,1) });

		// 6
		// xxx
		//  x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,1), Piece::build(1,1) });
		
		// 7
		// xxxx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0) });

		// 8
		// xxx
		// x  
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1) });

		// 9
		// xx
		//  xx  
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(1,2) });

		// 10
		// xxxxx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(4,0) });

		// 11
		// xxxx
		// x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(0,1) });

		// 12
		// xxx
		// x
		// x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1), Piece::build(0,2) });

		// 13
		// xxx
		//   xx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(2,1), Piece::build(3,1) });

		// 14
		// x
		// xxx
		//   x
		pieces.push_back({ Piece::build(0,0), Piece::build(0,1), Piece::build(1,1), Piece::build(2,1), Piece::build(2,2) });

		// 15
		// x
		// xx
		// xx
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(1,0), Piece::build(1,1) });

		// 16
		// xx
		//  xx
		//   x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(2,1), Piece::build(2,2) });

		// 17
		// xxx
		// x x 
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(0,1), Piece::build(2,1) });

		// 18
		//  x 
		// xxx
		//  x
		pieces.push_back({ Piece::build(1,0), Piece::build(0,1), Piece::build(1,1), Piece::build(2,1), Piece::build(1,2) });

		// 19
		// xxxx
		//  x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(2,0), Piece::build(3,0), Piece::build(1,1) });

		// 20
		// xx
		//  xx
		//  x
		pieces.push_back({ Piece::build(0,0), Piece::build(1,0), Piece::build(1,1), Piece::build(2,1), Piece::build(1,2) });


		for (Piece& piece : pieces)
			piece.sort();

		return pieces;
	}

	//-------------------------------------------------------------------------------------------------
	PieceSymetries Helpers::getAllPieceSymetries()
	{
		std::vector<Piece> pieces = getAllPieces();
		PieceSymetries symetries(pieces.size());

		u32 i = 0;
		for (const Piece& p : pieces)
		{
			for (bool flip : {false, true})
			{
				for (Rotation rot : { Rotation::Rot_0, Rotation::Rot_90, Rotation::Rot_180, Rotation::Rot_270 })
				{
					Piece rotatedP = p.rotate(flip ? Rotation::Flip_X : Rotation::Rot_0).rotate(rot);
					rotatedP.sort();
					symetries[i].insert(rotatedP);
				}
			}

		    ++i;
		}

		return symetries;
	}
}