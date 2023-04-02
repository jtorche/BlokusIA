#include "AI/blockusAI/BlokusGame.h"
#include "AI/blockusAI/BlokusGameHelpers.h"

namespace blokusAI
{
	void cornerTest()
	{

		// Corner test
		Corners corners;
		for (u32 i = 0; i < 4; ++i)
		{
			corners.setCorners(i, 1, 0, 0, 1);

			ubyte gettedCorners[4];
			corners.getCorners(i, gettedCorners);
			DEBUG_ASSERT(gettedCorners[0] == 1 && gettedCorners[1] == 0 && gettedCorners[2] == 0 && gettedCorners[3] == 1);
		}
	}

	void pieceTest()
	{
		{
			Piece::Tile tile1 = Piece::build(0, 0);
			DEBUG_ASSERT(Piece::getTileX(tile1) == 0 && Piece::getTileY(tile1) == 0);

			Piece::Tile tile2 = Piece::build(1, 0);
			DEBUG_ASSERT(Piece::getTileX(tile2) == 1 && Piece::getTileY(tile2) == 0);

			Piece::Tile tile3 = Piece::build(0, 1);
			DEBUG_ASSERT(Piece::getTileX(tile3) == 0 && Piece::getTileY(tile3) == 1);
		}
		{
			Piece piece(Piece::build(0, 0));
			ubyte corners[4];
			piece.getCorners(0, corners);
			DEBUG_ASSERT(corners[0] == corners[1] == corners[2] == corners[3] == 1);
		}
		{
			Piece piece(Piece::build(0, 0), Piece::build(1, 0));
			ubyte corners[4];

			piece.getCorners(0, corners);
			DEBUG_ASSERT(corners[0] == 1 && corners[3] == 1 && corners[1] == 0 && corners[2] == 0);

			piece.getCorners(1, corners);
			DEBUG_ASSERT(corners[0] == 0 && corners[3] == 0 && corners[1] == 1 && corners[2] == 1);
		}
		{
			Piece piece(Piece::build(0, 0), Piece::build(0, 1));
			ubyte corners[4];

			piece.getCorners(0, corners);
			DEBUG_ASSERT(corners[0] == 1 && corners[1] == 1 && corners[2] == 0 && corners[3] == 0);

			piece.getCorners(1, corners);
			DEBUG_ASSERT(corners[0] == 0 && corners[1] == 0 && corners[2] == 1 && corners[3] == 1);
		}
		{
			Piece piece1(Piece::build(0, 0), Piece::build(1, 0));
			Piece piece2(Piece::build(1, 0), Piece::build(0, 0));

			DEBUG_ASSERT(piece1 == piece2);
		}
	}

	void pieceTest2()
	{
		auto cmp = [](Piece p1, Piece p2)
		{
			DEBUG_ASSERT(p1 == p2);
		};

		{
			Piece piece1(Piece::build(0, 0), Piece::build(1, 0));
			Piece piece2(Piece::build(0, 0), Piece::build(0, 1));

			cmp(piece1.rotate(Rotation::Rot_90), piece2);
			cmp(piece1.rotate(Rotation::Rot_180), piece1);
			cmp(piece1.rotate(Rotation::Rot_270), piece2);
		}
		{
			Piece piece(Piece::build(0, 0), Piece::build(1, 0), Piece::build(0, 1));
			cmp(piece.rotate(Rotation::Rot_180)
				.rotate(Rotation::Rot_180), piece);
			cmp(piece.rotate(Rotation::Rot_90)
				.rotate(Rotation::Rot_90)
				.rotate(Rotation::Rot_180), piece);
			cmp(piece.rotate(Rotation::Rot_90)
				.rotate(Rotation::Rot_270), piece);
		}

		{
			Piece piece(Piece::build(0, 0), Piece::build(1, 0), Piece::build(0, 1));
			Piece pieceFlipped(Piece::build(0, 0), Piece::build(1, 0), Piece::build(1, 1));
			cmp(piece.rotate(Rotation::Flip_X), pieceFlipped);
			cmp(pieceFlipped.rotate(Rotation::Rot_270), piece);
		}
	}

	void boardTest()
	{
		Piece p(Piece::build(0, 0));
		{
			Board board;
			board.addPiece(Slot::P0, p, { 0,0 });
			DEBUG_ASSERT(board.getSlot(0, 0) == Slot::P0);
		}
		{
			Board board;
			board.addPiece(Slot::P0, p, { 8,10 });
			board.addPiece(Slot::P1, p, { 9, 10 });
			board.addPiece(Slot::P2, p, { 10, 10 });
			board.addPiece(Slot::P3, p, { 11, 10 });
			DEBUG_ASSERT(board.getSlot(8, 10) == Slot::P0);
			DEBUG_ASSERT(board.getSlot(9, 10) == Slot::P1);
			DEBUG_ASSERT(board.getSlot(10, 10) == Slot::P2);
			DEBUG_ASSERT(board.getSlot(11, 10) == Slot::P3);
		}
	}
}

void runTest()
{
	blokusAI::cornerTest();
	blokusAI::pieceTest();
	blokusAI::pieceTest2();
	blokusAI::boardTest();

	std::cout << "Unit tests succeeded" << std::endl << std::endl;
	system("pause");
}
