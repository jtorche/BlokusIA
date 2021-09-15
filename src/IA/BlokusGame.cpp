#include "IA/BlokusGame.h"

#include <numeric>

namespace BlokusIA
{
	//-------------------------------------------------------------------------------------------------
	Piece::Piece(Tile _t0, Tile _t1, Tile _t2, Tile _t3, Tile _t4)
		: m_layout{ _t0, _t1, _t2, _t3, _t4 }
	{
		flush();
	}

	//-------------------------------------------------------------------------------------------------
	bool Piece::operator==(const Piece& _other) const
	{
		for (u32 i = 0; i < MaxTile; ++i)
			if (m_layout[i] != _other.m_layout[i])
				return false;

		DEBUG_ASSERT(m_corners == _other.m_corners);
		return true;
	}

	//-------------------------------------------------------------------------------------------------
	void Piece::flush()
	{
		m_numCorners = 0;
		m_numTiles = 0;

		for (u32 i = 0; i < MaxTile; ++i)
		{
			if (m_layout[i] != 0)
			{
				++m_numTiles;

				u32 curX = getTileX(m_layout[i]);
				u32 curY = getTileY(m_layout[i]);
				ubyte curCorners[4] = { 1,1,1,1 };

				for (u32 j = 0; j < MaxTile; ++j)
				{
					if (i != j && m_layout[j] != 0)
					{
						if (curY == getTileY(m_layout[j]))
						{
							if (curX == getTileX(m_layout[j]) + 1)
								curCorners[0] = curCorners[3] = 0;
							else if(curX + 1 == getTileX(m_layout[j]))
								curCorners[1] = curCorners[2] = 0;
						}
						else if (curX == getTileX(m_layout[j]))
						{
							if (curY == getTileY(m_layout[j]) + 1)
								curCorners[0] = curCorners[1] = 0;
							else if (curY + 1 == getTileY(m_layout[j]))
								curCorners[2] = curCorners[3] = 0;
						}
					}
				}

				m_corners.setCorners(i, curCorners[0], curCorners[1], curCorners[2], curCorners[3]);
				m_numCorners += std::accumulate(std::begin(curCorners), std::end(curCorners), ubyte(0), std::plus<ubyte>{});
			}
		}

		sort();
	}

	//-------------------------------------------------------------------------------------------------
	void Piece::sort()
	{
		u32 permutation[5] = { 0,1,2,3,4 };
		std::stable_sort(std::begin(permutation), std::end(permutation),
			[this](u32 a, u32 b)
			{
				if (m_layout[a] == 0 && m_layout[b] == 0)
					return a < b;
				else if(m_layout[a] == 0 || m_layout[b] == 0)
					return m_layout[a] != 0;
				else
				{
					if (getTileX(m_layout[a]) != getTileX(m_layout[b]))
						return getTileX(m_layout[a]) < getTileX(m_layout[b]);
					else
						return getTileY(m_layout[a]) < getTileY(m_layout[b]);
				}
			});

		Piece cpy(*this);
		for (u32 i = 0; i < MaxTile; ++i)
		{
			m_layout[i] = cpy.m_layout[permutation[i]];

			ubyte corners[4];
			cpy.getCorners(permutation[i], corners);
			m_corners.setCorners(i, corners[0], corners[1], corners[2], corners[3]);
		}
	}

	//-------------------------------------------------------------------------------------------------
	Piece Piece::rotate(Rotation _rot) const
	{
		if (_rot == Rotation::Rot_0)
			return *this;

		u32 maxX = 0, maxY= 0;
		for (u32 i = 0; i < MaxTile; ++i)
		{
			maxX = std::max(maxX, getTileX(m_layout[i]));
			maxY = std::max(maxY, getTileY(m_layout[i]));
		}

		Piece piece;
		for (u32 i = 0; i < MaxTile; ++i)
		{
			if (m_layout[i] == 0)
				break;

			u32 x = getTileX(m_layout[i]);
			u32 y = getTileY(m_layout[i]);

			ubyte corners[4];
			m_corners.getCorners(i, corners);

			switch (_rot)
			{
			case Rotation::Rot_90:
				piece.m_layout[i] = build(maxY - y, x);
				piece.m_corners.setCorners(i, corners[3], corners[0], corners[1], corners[2]);
				break;

			case Rotation::Rot_180:
				piece.m_layout[i] = build(maxX - x, maxY - y);
				piece.m_corners.setCorners(i, corners[2], corners[3], corners[0], corners[1]);
				break;

			case Rotation::Rot_270:
				piece.m_layout[i] = build(y, maxX - x);
				piece.m_corners.setCorners(i, corners[1], corners[2], corners[3], corners[0]);
				break;

			case Rotation::Flip_X:
				piece.m_layout[i] = build(maxX - x, y);
				piece.m_corners.setCorners(i, corners[1], corners[0], corners[3], corners[2]);
				break;
			}
		}

		piece.flush();
		return piece;
	}

	//-------------------------------------------------------------------------------------------------
	void Corners::setCorners(u32 _tileIndex, ubyte _c1, ubyte _c2, ubyte _c3, ubyte _c4)
	{
		DEBUG_ASSERT(_tileIndex < Piece::MaxTile);
		DEBUG_ASSERT(_c1 == 0 || _c1 == 1);
		DEBUG_ASSERT(_c2 == 0 || _c2 == 1);
		DEBUG_ASSERT(_c3 == 0 || _c3 == 1);
		DEBUG_ASSERT(_c4 == 0 || _c4 == 1);

		if((_tileIndex % 2) == 0)
			m_data[_tileIndex / 2] = _c1 + (_c2 << 1) + (_c3 << 2) + (_c4 << 3) + (m_data[_tileIndex / 2] & 0xF0);
		else
			m_data[_tileIndex / 2] = (m_data[_tileIndex / 2] & 0xF) + (_c1 << 4) + (_c2 << 5) + (_c3 << 6) + (_c4 << 7);
	}

	//-------------------------------------------------------------------------------------------------
	void Corners::getCorners(u32 _tileIndex, ubyte(&_c)[4]) const
	{
		DEBUG_ASSERT(_tileIndex < Piece::MaxTile);

		for (u32 i = 0; i < 4; ++i)
			_c[i] = (m_data[_tileIndex / 2] >> (i + ((_tileIndex % 2)*4))) & 0x1;
	}
	
	//-------------------------------------------------------------------------------------------------
	void Board::print() const
	{
		for (u32 j = 0; j < BoardSize; ++j)
		{
			for (u32 i = 0; i < BoardSize; ++i)
			{
				std::cout << (u32)getSlot(i, j) << " ";
			}
			std::cout << std::endl;
		}
	}

	//-------------------------------------------------------------------------------------------------
	static u32 flatten(u32 i, u32 j) { return (j * Board::BoardSize + i); }

	//-------------------------------------------------------------------------------------------------
	Slot Board::getSlot(u32 _x, u32 _y) const
	{
	    u32 packed = m_board[flatten(_x, _y) >> 3];
		u32 offset = (flatten(_x, _y) & 0x7) << 2;
		return Slot((packed >> offset) & 0xF);
	}

	//-------------------------------------------------------------------------------------------------
	void Board::setSlot(u32 _x, u32 _y, Slot _slot)
	{
		u32 packed = m_board[flatten(_x, _y) >> 3];
		u32 offset = (flatten(_x, _y) & 0x7) << 2;
		packed = packed & ~(0xF << offset);
		packed += u32(_slot) << offset;
		m_board[flatten(_x, _y) >> 3] = packed;
	}

	//-------------------------------------------------------------------------------------------------
	Slot Board::getSlotSafe(i32 _x, i32 _y) const
	{
		if (_x < 0 || _x >= i32(BoardSize) || _y < 0 || _y >= i32(BoardSize))
			return Slot::Empty;
		return getSlot(u32(_x), u32(_y));
	}

	//-------------------------------------------------------------------------------------------------
	bool Board::canAddPiece(Slot _player, const Piece& _piece, uvec2 _pos, u32* _adjacencyScore) const
	{
		DEBUG_ASSERT(_player != Slot::Empty);
		DEBUG_ASSERT(_pos.x < BoardSize && _pos.y < BoardSize);

		for (u32 i = 0; i < Piece::MaxTile; ++i)
		{
			if (_piece.getTile(i) == 0)
				break;

			u32 tileX = Piece::getTileX(_piece.getTile(i)) + _pos.x;
			u32 tileY = Piece::getTileY(_piece.getTile(i)) + _pos.y;

			if (tileX >= BoardSize || tileY >= BoardSize)
				return false;

			if (getSlot(tileX, tileY) != Slot::Empty)
				return false;

			// Implement the non contact rule with pieces of the same player
			i32 iTileX = i32(tileX);
			i32 iTileY = i32(tileY);

            Slot s = getSlotSafe(iTileX - 1, iTileY);
			if (s == _player) return false;
            if (_adjacencyScore) _adjacencyScore += (s == Slot::Empty ? 0 : 1);

            s = getSlotSafe(iTileX + 1, iTileY);
            if (s == _player) return false;
            if (_adjacencyScore) _adjacencyScore += (s == Slot::Empty ? 0 : 1);

            s = getSlotSafe(iTileX, iTileY - 1);
            if (s == _player) return false;
            if (_adjacencyScore) _adjacencyScore += (s == Slot::Empty ? 0 : 1);

            s = getSlotSafe(iTileX, iTileY + 1);
            if (s == _player) return false;
            if (_adjacencyScore) _adjacencyScore += (s == Slot::Empty ? 0 : 1);
		}

		return true;
	}

	//-------------------------------------------------------------------------------------------------
	void Board::addPiece(Slot _player, const Piece& _piece, ubyte2 _pos)
	{
		DEBUG_ASSERT(_player != Slot::Empty);
		DEBUG_ASSERT(_pos.x < BoardSize && _pos.y < BoardSize);

		for (u32 i = 0; i < Piece::MaxTile; ++i)
		{
			if (_piece.getTile(i) == 0)
				break;

			u32 tileX = Piece::getTileX(_piece.getTile(i)) + _pos.x;
			u32 tileY = Piece::getTileY(_piece.getTile(i)) + _pos.y;

			setSlot(tileX, tileY, _player);
		}
	}

	//-------------------------------------------------------------------------------------------------
	u32 Board::computeValidSlotsForPlayer(Slot _player, Board::PlayableSlots& _result) const
	{
		DEBUG_ASSERT(_player != Slot::Empty);

		u32 numCorners = 0;
		for (i32 j = 0; j < i32(BoardSize); ++j)
		{
			for (i32 i = 0; i < i32(BoardSize); ++i)
			{
				if (getSlot(u32(i), u32(j)) == Slot::Empty)
				{
					if (getStartingPosition(_player) == uvec2{ u32(i), u32(j) })
					{
						_result[numCorners++] = { ubyte(i), ubyte(j) };
					}
					else
					{
						if (getSlotSafe(i - 1, j) != _player && getSlotSafe(i + 1, j) != _player &&
							getSlotSafe(i, j - 1) != _player && getSlotSafe(i, j + 1) != _player)
						{
							if (getSlotSafe(i - 1, j - 1) == _player || getSlotSafe(i + 1, j - 1) == _player ||
								getSlotSafe(i - 1, j + 1) == _player || getSlotSafe(i + 1, j + 1) == _player)
							{
								_result[numCorners++] = { ubyte(i), ubyte(j) };
							}
						}
					}
				}
			}
		}

		return numCorners;
	}

	//-------------------------------------------------------------------------------------------------
	uvec2 Board::getStartingPosition(Slot _player) const
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

	//-------------------------------------------------------------------------------------------------
	u32 Board::getPiecePlayablePositions(Slot _player, const Piece& _piece, ubyte2 _boardPos, bool _dontCheckCornerRule, 
                                         std::array<ubyte2, Piece::MaxPlayableCorners>& _outPositions, std::array<u32, Piece::MaxPlayableCorners>* _adjacencyScores) const
	{
		DEBUG_ASSERT(_player != Slot::Empty);
		DEBUG_ASSERT(_boardPos.x < BoardSize && _boardPos.y < BoardSize);

		u32 numPosition = 0;
		ivec2 iBoardPos = { int(_boardPos.x), int(_boardPos.y) };

		bool compatibleCorner[4] = 
		{ 
			getSlotSafe(iBoardPos.x - 1, iBoardPos.y - 1) == _player || _dontCheckCornerRule,
			getSlotSafe(iBoardPos.x + 1, iBoardPos.y - 1) == _player || _dontCheckCornerRule,
			getSlotSafe(iBoardPos.x + 1, iBoardPos.y + 1) == _player || _dontCheckCornerRule,
			getSlotSafe(iBoardPos.x - 1, iBoardPos.y + 1) == _player || _dontCheckCornerRule
		};

		for (u32 i = 0; i < Piece::MaxTile; ++i)
		{
			if (_piece.getTile(i) == 0)
				break;

			ivec2 tilePos = { int(Piece::getTileX(_piece.getTile(i))), int(Piece::getTileY(_piece.getTile(i))) };
			
			ubyte corners[4];
			_piece.getCorners(i, corners);

			for (u32 c = 0 ; c < 4 ; ++c)
			{
				if (corners[c] && compatibleCorner[c])
				{
					ivec2 finalPos = iBoardPos - tilePos;
					if (finalPos.x >= 0 && finalPos.y >= 0)
					{
                        u32 adjacencyScore = 0;
						if (canAddPiece(_player, _piece, uvec2(u32(finalPos.x), u32(finalPos.y)), _adjacencyScores ? &adjacencyScore : nullptr))
						{
                            if (_adjacencyScores)
                                _adjacencyScores->operator[](numPosition) = adjacencyScore;

							_outPositions[numPosition++] = { ubyte(finalPos.x), ubyte(finalPos.y) };
							break;
						}
					}
				}
			}
		}

		return numPosition;
	}
}