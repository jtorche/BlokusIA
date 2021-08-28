#pragma once

#include <iostream>
#include <array>
#include "timCore/type.h"
#include "timCore/Common.h"

namespace BlokusIA
{
	enum class Slot : ubyte
	{
		Empty,
		P0, P1, P2, P3
	};

	// Encode valid corners for a piece
	struct Corners
	{
		// we need 4 x 5 bits to encode the corners of a piece (at most 5 tiles)
		// A corner is used to connect the piece to another one. 
		ubyte m_data[3] = { 0,0,0 };

		// nth bit representing a nth corner in the following order:
		// 1    2
		// ------
		// |    |
		// |    |
		// ------
		// 4    3

		void setCorners(u32 _tileIndex, ubyte _c1, ubyte _c2, ubyte _c3, ubyte _c4);
		void getCorners(u32 _tileIndex, ubyte (&_c)[4]) const;
	};

	class Piece
	{
	public:
		// Tile definition
		using Tile = ubyte;

		static Tile build(u32 _x, u32 _y)
		{
			TIM_ASSERT(_x <= 5 && _y <= 5);
			return _x + ubyte(_y << 3) + 64;
		}

		Piece(Tile _t0, Tile _t1 = 0, Tile _t2 = 0, Tile _t3 = 0, Tile _t4 = 0) 
			: m_layout{ _t0, _t1, _t2, _t3, _t4 } 
		{
			generateCorners();
		}

		void getCorners(u32 _index, ubyte(&_c)[4]) const { return m_corners.getCorners(_index, _c); }
		Tile getTile(u32 _index) const { return m_layout[_index]; }

		static u32 getTileX(Tile _tile) { return _tile & 0x7; }
		static u32 getTileY(Tile _tile) { return (_tile >> 3) & 0x7; }

	private:
		void generateCorners();

		Tile m_layout[5];
		Corners m_corners;
	};

	class Board
	{
	public:
		static constexpr u32 BoardSize = 20;


		Board() = default;
		Board(const Board&) = default;

		void print() const;

	private:
		std::array<Slot, BoardSize*BoardSize> m_board = { { Slot::Empty } };

	private:
		static u32 flatten(u32 i, u32 j) { return j * BoardSize + i; }
	};
}