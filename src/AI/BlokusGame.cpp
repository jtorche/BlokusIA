#include <numeric>

#include "AI/BlokusGame.h"

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

        if (m_numTiles > 0)
        {
            sort();
            computeNumBorderTiles();
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
    void Piece::computeNumBorderTiles()
    {
        bool smallGrid[g_MaxPieceSizeX+2][g_MaxPieceSizeY+2] = { {} };
        
        for (u32 i = 0; i < MaxTile; ++i)
        {
            if (m_layout[i] != 0)
            {
                u32 curX = getTileX(m_layout[i]);
                u32 curY = getTileY(m_layout[i]);
                DEBUG_ASSERT(curX < g_MaxPieceSizeX && curY < g_MaxPieceSizeY);

                smallGrid[curX+1][curY+1] = true;
            }
        }

        m_numBorderTiles = 0;
        for(u32 i = 0 ; i < g_MaxPieceSizeX+2; ++i)
            for (u32 j = 0; j < g_MaxPieceSizeY+2; ++j)
        {
            if (!smallGrid[i][j])
            {
                if (i > 0 && smallGrid[i - 1][j])
                    m_numBorderTiles++;
                else if(i < g_MaxPieceSizeX + 1 && smallGrid[i + 1][j])
                    m_numBorderTiles++;
                else if (j > 0 && smallGrid[i][j - 1])
                    m_numBorderTiles++;
                else if (j < g_MaxPieceSizeY + 1 && smallGrid[i][j + 1])
                    m_numBorderTiles++;
            }
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
    void Piece::print() const
    {
        char grid[g_MaxPieceSizeX + 2][g_MaxPieceSizeY + 2];
        for (u32 j = 0; j < g_MaxPieceSizeY + 2; ++j)
            for(u32 i = 0 ; i< g_MaxPieceSizeX + 2; ++i)
        {
            grid[i][j] = ' ';
        }

        u32 maxY = 0;
        for (u32 i = 0; i < MaxTile; ++i)
        {
            if (m_layout[i] != 0)
            {
                u32 curX = getTileX(m_layout[i])+1;
                u32 curY = getTileY(m_layout[i])+1;
                maxY = std::max(maxY, curY);

                grid[curX][curY] = '0';

                ubyte corners[4];
                getCorners(i, corners);

                if(corners[0])
                    grid[curX-1][curY-1] = 'X';
                if (corners[1])
                    grid[curX+1][curY-1] = 'X';
                if (corners[2])
                    grid[curX+1][curY+1] = 'X';
                if (corners[3])
                    grid[curX-1][curY+1] = 'X';
            }
        }

        for (u32 j = 0; j < maxY+2; ++j)
        {
            for (u32 i = 0; i < g_MaxPieceSizeX + 2; ++i)
            {
                std::cout << grid[i][j];
            }
            std::cout << std::endl;
        }
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
    bool Board::operator==(const Board& _board) const
    {
        return memcmp(m_board.data(), _board.m_board.data(), sizeof(m_board)) == 0;
    }

    //-------------------------------------------------------------------------------------------------
    void Board::setSlot(u32 _x, u32 _y, Slot _slot)
    {
        DEBUG_ASSERT(_slot != Slot::Empty);

        u32 flattenIndex = flatten(_x, _y) >> 3;
        u32 packed = m_board[flattenIndex];
        u32 offset = (flatten(_x, _y) & 0x7) << 2;
        packed = packed & ~(0xF << offset);
        packed += u32(_slot) << offset;
        m_board[flattenIndex] = packed;

        auto updateContactRuleCache = [this, _slot](u32 x, u32 y)
        {
            u32 flattenIndex = flatten(x, y) >> 3;
            u32 packed = m_nonContactRuleCache[flattenIndex];
            u32 offset = (flatten(x, y) & 0x7) << 2;
            u32 mask = 1 << (u32(_slot) - u32(Slot::P0));
            packed = packed | (mask << offset);
            m_nonContactRuleCache[flattenIndex] = packed;
        };

        updateContactRuleCache(_x, _y);
        if (_x > 0)             updateContactRuleCache(_x - 1, _y);
        if (_x < BoardSize - 1) updateContactRuleCache(_x + 1, _y);
        if (_y > 0)             updateContactRuleCache(_x, _y - 1);
        if (_y < BoardSize - 1) updateContactRuleCache(_x, _y + 1);
    }

	//-------------------------------------------------------------------------------------------------
	bool Board::canAddPiece(Slot _player, const Piece& _piece, uvec2 _pos) const
	{
		DEBUG_ASSERT(_player != Slot::Empty);
		DEBUG_ASSERT(_pos.x < BoardSize && _pos.y < BoardSize);

        ubyte playerIndex = ubyte(_player) - ubyte(Slot::P0);
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

            if (getContactRuleCache(tileX, tileY) & (ubyte(1) << playerIndex))
                return false;
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
    bool Board::isValidPlayableSlot(Slot _player, ubyte2 _pos) const
    {
        if (getSlot(_pos.x, _pos.y) == Slot::Empty)
        {
            int x = _pos.x;
            int y = _pos.y;

            return (getSlotSafe<-1, 0>(x, y) != _player && getSlotSafe<1, 0>(x, y) != _player &&
                    getSlotSafe<0, -1>(x, y) != _player && getSlotSafe<0, 1>(x, y) != _player &&
                    (getSlotSafe<-1, -1>(x, y) == _player || getSlotSafe<1, -1>(x, y) == _player ||
                     getSlotSafe<-1, 1>(x, y) == _player || getSlotSafe<1, 1>(x, y) == _player));
        }
        return false;
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
                ubyte2 pos = ubyte2{ ubyte(i), ubyte(j) };
                if ((getStartingPosition(_player) == pos && getSlot(pos.x, pos.y) == Slot::Empty) || isValidPlayableSlot(_player, pos))
                {
                    _result[numCorners++] = pos;
                }
			}
		}

		return numCorners;
	}

	//-------------------------------------------------------------------------------------------------
	u32 Board::getPiecePlayablePositions(Slot _player, const Piece& _piece, ubyte2 _boardPos, std::array<ubyte2, Piece::MaxPlayableCorners>& _outPositions, bool _isFirstMove) const
	{
		DEBUG_ASSERT(_player != Slot::Empty);
		DEBUG_ASSERT(_boardPos.x < BoardSize&& _boardPos.y < BoardSize);

		u32 numPosition = 0;
		ivec2 iBoardPos = { int(_boardPos.x), int(_boardPos.y) };

		bool compatibleCorner[4] = 
		{ 
			getSlotSafe<-1,-1>(iBoardPos.x, iBoardPos.y) == _player || _isFirstMove,
			getSlotSafe<1,-1>(iBoardPos.x, iBoardPos.y) == _player || _isFirstMove,
			getSlotSafe<1,1>(iBoardPos.x, iBoardPos.y) == _player || _isFirstMove,
			getSlotSafe<-1,1>(iBoardPos.x, iBoardPos.y) == _player || _isFirstMove
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
						if (canAddPiece(_player, _piece, uvec2(u32(finalPos.x), u32(finalPos.y))))
						{
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