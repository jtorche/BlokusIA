#include <iostream>
#include <array>
#include "timCore/type.h"
#include "timCore/Common.h"

#include "BlokusGame.h"

void runTest();

using namespace BlokusIA;

int main()
{
	// run some unit test
	runTest();

	Board b;
	auto pieces = Piece::getAllPieces();
	for (const Piece& p : pieces)
	{
		while (1)
		{
			u32 x = rand() % Board::BoardSize;
			u32 y = rand() % Board::BoardSize;

			if (b.canAddPiece(Slot::P0, p, { x,y }))
			{
				b.addPiece(Slot::P0, p, { x,y });
				break;
			}
		}
	}
	b.print();
	return 0;
}