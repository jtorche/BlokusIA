#include "BlokusGame.h"

namespace BlokusIA
{
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

	//-------------------------------------------------------------------------------------------------
	void Piece::generateCorners()
	{
		for (u32 i = 0; i < 5; ++i)
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
}