#include "BlokusGame.h"

namespace BlokusIA
{
	bool Piece::operator==(const Piece& _other) const
	{
		for (u32 i = 0; i < MaxTile; ++i)
			if (m_layout[i] != _other.m_layout[i])
				return false;

		return m_corners == _other.m_corners;
	}

	//-------------------------------------------------------------------------------------------------
	void Piece::generateCorners()
	{
		for (u32 i = 0; i < MaxTile; ++i)
		{
			if (m_layout[i] != 0)
			{
				u32 curX = getTileX(m_layout[i]);
				u32 curY = getTileY(m_layout[i]);
				ubyte curCorners[4] = { 1,1,1,1 };

				for (u32 j = 0; j < 5; ++j)
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
			}
		}
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

			ubyte x = getTileX(m_layout[i]);
			ubyte y = getTileY(m_layout[i]);

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
			case  Rotation::Rot_270:
				piece.m_layout[i] = build(y, maxX - x);
				piece.m_corners.setCorners(i, corners[1], corners[2], corners[3], corners[0]);
				break;
			}


		}

		return piece;
	}

	//-------------------------------------------------------------------------------------------------
	void Corners::setCorners(u32 _tileIndex, ubyte _c1, ubyte _c2, ubyte _c3, ubyte _c4)
	{
		if((_tileIndex % 2) == 0)
			m_data[_tileIndex / 2] = _c1 + (_c2 << 1) + (_c3 << 2) + (_c4 << 3) + (m_data[_tileIndex / 2] & 0xF0);
		else
			m_data[_tileIndex / 2] = (m_data[_tileIndex / 2] & 0xF) + (_c1 << 4) + (_c2 << 5) + (_c3 << 6) + (_c4 << 7);
	}

	//-------------------------------------------------------------------------------------------------
	void Corners::getCorners(u32 _tileIndex, ubyte(&_c)[4]) const
	{
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
				std::cout << (u32)m_board[flatten(i, j)] << " ";
			}
			std::cout << std::endl;
		}
	}

	Slot Board::getSlotSafe(i32 _x, i32 _y) const
	{
		if (_x < 0 || _x >= i32(BoardSize) || _y < 0 || _y >= i32(BoardSize))
			return Slot::Empty;
		return getSlot(u32(_x), u32(_y));
	}

	//-------------------------------------------------------------------------------------------------
	bool Board::canAddPiece(Slot _player, const Piece& _piece, uvec2 _pos) const
	{
		TIM_ASSERT(_player != Slot::Empty);
		TIM_ASSERT(_pos.x < BoardSize && _pos.y < BoardSize);

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
			if (getSlotSafe(iTileX - 1, iTileY) == _player)
				return false;
			if (getSlotSafe(iTileX + 1, iTileY) == _player)
				return false;
			if (getSlotSafe(iTileX, iTileY - 1) == _player)
				return false;
			if (getSlotSafe(iTileX, iTileY + 1) == _player)
				return false;
		}

		return true;
	}
}