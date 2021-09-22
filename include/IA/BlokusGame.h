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
			return ubyte(_x) + ubyte(_y << 3) + ubyte(64); // Flag 64 is to distinguish Tile in (0, 0) and unused Tile
		}

		Piece(Tile _t0 = 0, Tile _t1 = 0, Tile _t2 = 0, Tile _t3 = 0, Tile _t4 = 0);

		Piece(const Piece&) = default;
		Piece& operator=(const Piece&) = default;

		bool operator==(const Piece& _other) const;

		void getCorners(u32 _index, ubyte(&_c)[4]) const { return m_corners.getCorners(_index, _c); }
		Tile getTile(u32 _index) const { return m_layout[_index]; }
		ubyte getNumTiles() const { return m_numTiles; }
		ubyte getNumCorners() const { return m_numCorners; }
        u16 getNumBorderTiles() const { return m_numBorderTiles; }

		Piece rotate(Rotation _rot) const;

		static u32 getTileX(Tile _tile) { return _tile & 0x7; }
		static u32 getTileY(Tile _tile) { return (_tile >> 3) & 0x7; }

	private:
		void sort();
		void flush();
        void computeNumBorderTiles();

		Tile m_layout[MaxTile] = { {0} };
		Corners m_corners;

		ubyte m_numTiles = 0;
		ubyte m_numCorners = 0;
        u16 m_numBorderTiles = 0;

	};
	static_assert(sizeof(Piece) == 12);

	//-------------------------------------------------------------------------------------------------
	class Board
	{
	public:
		static constexpr u32 BoardSize = 20;
		static constexpr u32 MaxPlayableCorners = 64; // arbitrary value, probably too high

		Board() = default;
		Board(const Board&) = default;

        bool operator==(const Board&) const;

		Slot getSlot(u32 _x, u32 _y) const;
		void setSlot(u32 _x, u32 _y, Slot _slot);
        template<int OffsetX, int OffsetY> Slot getSlotSafe(i32 _x, i32 _y) const;

		bool canAddPiece(Slot _player, const Piece& _piece, uvec2 _pos) const;
		void addPiece(Slot _player, const Piece& _piece, ubyte2 _pos);

		static ubyte2 getStartingPosition(Slot _player);

		using PlayableSlots = std::array<ubyte2, MaxPlayableCorners>;
		u32 computeValidSlotsForPlayer(Slot _player, PlayableSlots& _result) const;
        bool isValidPlayableSlot(Slot _player, ubyte2 _pos) const;

		// Assuming _boardPos is a valid position from "computeValidSlotsForPlayer"
		u32 getPiecePlayablePositions(Slot _player, const Piece& _piece, ubyte2 _boardPos, std::array<ubyte2, Piece::MaxPlayableCorners>&, bool _isFirstMove) const;


		void print() const;

	private:
		static_assert((BoardSize*BoardSize) % 8 == 0);

		// each u32 store a 8x1 sub board, each slot on 4 bits
		std::array<u32, (BoardSize*BoardSize)/8> m_board = { {0} };

        friend std::hash<BlokusIA::Board>;
	};

	//-------------------------------------------------------------------------------------------------
	struct BlokusGame
	{
		static constexpr u32 PiecesCount = 21;
	};

//-------------------------------------------------------------------------------------------------
//---- Inline function
//-------------------------------------------------------------------------------------------------
    static u32 flatten(u32 i, u32 j) { return (j * Board::BoardSize + i); }

    //-------------------------------------------------------------------------------------------------
    inline Slot Board::getSlot(u32 _x, u32 _y) const
    {
        u32 packed = m_board[flatten(_x, _y) >> 3];
        u32 offset = (flatten(_x, _y) & 0x7) << 2;
        return Slot((packed >> offset) & 0xF);
    }

    //-------------------------------------------------------------------------------------------------
    inline void Board::setSlot(u32 _x, u32 _y, Slot _slot)
    {
        u32 packed = m_board[flatten(_x, _y) >> 3];
        u32 offset = (flatten(_x, _y) & 0x7) << 2;
        packed = packed & ~(0xF << offset);
        packed += u32(_slot) << offset;
        m_board[flatten(_x, _y) >> 3] = packed;
    }

    //-------------------------------------------------------------------------------------------------
    template<int OffsetX, int OffsetY>
    Slot Board::getSlotSafe(i32 _x, i32 _y) const
    {
        if constexpr (OffsetX < 0 && OffsetY < 0)
        {
            if (_x + OffsetX < 0 || _y + OffsetY < 0)
                return Slot::Empty;
        }
        else if constexpr (OffsetX > 0 && OffsetY < 0)
        {
            if (_x + OffsetX >= i32(BoardSize) || _y + OffsetY < 0)
                return Slot::Empty;
        }
        else if constexpr (OffsetX < 0 && OffsetY > 0)
        {
            if (_x + OffsetX < 0 || _y + OffsetY >= i32(BoardSize))
                return Slot::Empty;
        }
        else if constexpr (OffsetX > 0 && OffsetY > 0)
        {
            if (_x + OffsetX >= i32(BoardSize) || _y + OffsetY >= i32(BoardSize))
                return Slot::Empty;
        }

        else if constexpr (OffsetX < 0) 
        {
            if(_x + OffsetX < 0)
                return Slot::Empty;
        }
        else if constexpr (OffsetX > 0)
        {
            if (_x + OffsetX >= i32(BoardSize))
                return Slot::Empty;
        }
        else if constexpr (OffsetY < 0)
        {
            if (_y + OffsetY < 0)
                return Slot::Empty;
        }
        else if constexpr (OffsetY > 0)
        {
            if (_y + OffsetY >= i32(BoardSize))
                return Slot::Empty;
        }
        
        return getSlot(u32(_x+OffsetX), u32(_y+OffsetY));
    }

    //-------------------------------------------------------------------------------------------------
    inline ubyte2 Board::getStartingPosition(Slot _player)
    {
        switch (_player)
        {
        case Slot::P0:
            return { 0,0 };
        case Slot::P1:
            return { 0, BoardSize - 1 };
        case Slot::P2:
            return { BoardSize - 1, BoardSize - 1 };
        case Slot::P3:
            return { BoardSize - 1, 0 };
        }

        return {};
    }
}

//-------------------------------------------------------------------------------------------------
//---- Hash function inside std namespace
//-------------------------------------------------------------------------------------------------
namespace std
{
	template<> struct hash<BlokusIA::Piece>
	{
		size_t operator()(const BlokusIA::Piece& _key) const
		{
			size_t h = 0;
			for (u32 i = 0; i < BlokusIA::Piece::MaxTile; ++i)
			{
				core::hash_combine(h, _key.getTile(i));
				if (_key.getTile(i) == 0)
					break;
			}
			return h;
		}
	};

    template<> struct hash<BlokusIA::Board>
    {
        size_t operator()(const BlokusIA::Board& _key) const
        {
            size_t h = 0;
            for(u32 dat : _key.m_board)
                core::hash_combine(h, dat);
            return h;
        }
    };
}