#pragma once

#include <array>
#include <iostream>
#include <vector>

#include "Core/Common.h"
#include "Core/hash.h"
#include "Core/type.h"

namespace BlokusIA
{
	enum class Slot : ubyte
	{
		Empty,
		P0, P1, P2, P3
	};

	struct PackedSlot
	{
		Slot p1 :	2;
		Slot p2 :	2;
		Slot p3 :	2;
		ubyte pad : 2;
	};

	static_assert(sizeof(PackedSlot) == 1);

	enum class Rotation // clock wise rotation of a piece
	{
		Rot_0,
		Rot_90,
		Rot_180,
		Rot_270,
		Flip_X
	};

	// Encode valid corners for a piece
	struct Corners
	{
		// we need 4 x 5 bits to encode the corners of a piece (at most 5 tiles)
		// A corner is used to connect the piece to another one. 
		ubyte m_data[3] = { {0} };

		// nth bit representing a nth corner in the following order:
		// 1    2
		// ------
		// |    |
		// |    |
		// ------
		// 4    3

		void setCorners(u32 _tileIndex, ubyte _c1, ubyte _c2, ubyte _c3, ubyte _c4);
		void getCorners(u32 _tileIndex, ubyte (&_c)[4]) const;

		bool operator==(const Corners& _other) const
		{
			return m_data[0] == _other.m_data[0] && m_data[1] == _other.m_data[1] && m_data[2] == _other.m_data[2];
		}
	};

	//-------------------------------------------------------------------------------------------------
	class Piece
	{
	public:
		// Tile definition
		using Tile = ubyte;
		static constexpr u32 MaxTile = 5;
		static constexpr u32 MaxPlayableCorners = 8;

		static Tile build(u32 _x, u32 _y)
		{
			DEBUG_ASSERT(_x < MaxTile && _y < MaxTile);
			return _x + ubyte(_y << 3) + 64 /* To distinguish Tile in (0, 0) and unused Tile */;
		}

		Piece(Tile _t0 = 0, Tile _t1 = 0, Tile _t2 = 0, Tile _t3 = 0, Tile _t4 = 0);

		Piece(const Piece&) = default;
		Piece& operator=(const Piece&) = default;

		bool operator==(const Piece& _other) const;

		void getCorners(u32 _index, ubyte(&_c)[4]) const { return m_corners.getCorners(_index, _c); }
		Tile getTile(u32 _index) const { return m_layout[_index]; }
		ubyte getNumTiles() const { return m_numTiles; }
		ubyte getNumCorners() const { return m_numCorners; }

		Piece rotate(Rotation _rot) const;

		static u32 getTileX(Tile _tile) { return _tile & 0x7; }
		static u32 getTileY(Tile _tile) { return (_tile >> 3) & 0x7; }

	private:
		void sort();
		void flush();

		Tile m_layout[MaxTile] = { {0} };
		Corners m_corners;

		ubyte m_numTiles = 0;
		ubyte m_numCorners = 0;
		ubyte2 m_pad;

	};
	static_assert(sizeof(Piece) == 12);

	//-------------------------------------------------------------------------------------------------
	class Board
	{
	public:
		static constexpr u32 BoardSize = 20;
		static constexpr u32 MaxPlayableCorners = 128; // arbitrary value, probably too high

		Board() = default;
		Board(const Board&) = default;

		Slot getSlot(u32 _x, u32 _y) const { return m_board[flatten(_x, _y)]; }
		Slot getSlotSafe(i32 _x, i32 _y) const;

		bool canAddPiece(Slot _player, const Piece& _piece, uvec2 _pos) const;
		void addPiece(Slot _player, const Piece& _piece, ubyte2 _pos);

		uvec2 getStartingPosition(Slot _player) const;

		using PlayableSlots = std::array<ubyte2, MaxPlayableCorners>;
		u32 computeValidSlotsForPlayer(Slot _player, PlayableSlots& _result) const;

		// Assuming _boardPos is a valid position from "computeValidSlotsForPlayer"
		u32 getPiecePlayablePositions(Slot _player, const Piece& _piece, ubyte2 _boardPos, std::array<ubyte2, Piece::MaxPlayableCorners>&, bool _isFirstMove) const;


		void print() const;

	private:
		std::array<Slot, BoardSize*BoardSize> m_board = { { Slot::Empty } };

	private:
		static u32 flatten(u32 i, u32 j) { return j * BoardSize + i; }
	};

	//-------------------------------------------------------------------------------------------------
	struct BlokusGame
	{
		static constexpr u32 PiecesCount = 21;
	};
}

//-------------------------------------------------------------------------------------------------
namespace std
{
	template<> struct hash<BlokusIA::Piece>
	{
		size_t operator()(const BlokusIA::Piece& key) const
		{
			size_t h = 0;
			for (u32 i = 0; i < BlokusIA::Piece::MaxTile; ++i)
			{
				core::hash_combine(h, key.getTile(i));
				if (key.getTile(i) == 0)
					break;
			}
			return h;
		}
	};
}