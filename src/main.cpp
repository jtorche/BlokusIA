#include <iostream>
#include <array>
#include "timCore/type.h"
#include "timCore/Common.h"

#include "BlokusIA.h"
#include "BlokusGameHelpers.h"

void runTest();

using namespace BlokusIA;

int main()
{
	// run some unit test
	runTest();

	initBlokusIA();

	GameState gameState;
	auto moves = gameState.enumerateMoves();

	Board board;
	auto pieces = Helpers::getAllPieces();

	bool first = true;
	for (const auto& p : pieces)
	{
		Board::PlayableSlots playableSlots;
		u32 playableSlotsCount = board.computeValidSlotsForPlayer(Slot::P0, playableSlots);

		for (u32 i = 0; i < playableSlotsCount; ++i)
		{
			std::array<ubyte2, Piece::MaxPlayableCorners> positions;
			u32 numPosition = board.getPiecePlayablePositions(Slot::P0, p, playableSlots[i], positions, first);
			
			if (numPosition > 0)
			{
				board.addPiece(Slot::P0, p, positions[0]);
				break;
			}
		}
		board.print();
		system("pause");
		first = false;
	}

	return 0;
}