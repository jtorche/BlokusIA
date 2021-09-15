#include "IA/BlokusIA.h"
#include "IA/BlokusGameHelpers.h"
#include <numeric>

namespace BlokusIA
{
	PieceSymetries s_allPieces = {};
    u32 s_totalPieceTileCount = 0;

	//-------------------------------------------------------------------------------------------------
	void initBlokusIA()
	{
		s_allPieces = Helpers::getAllPieceSymetries();
        s_totalPieceTileCount = 0;
        for (const auto& _pieces : s_allPieces)
        {
            s_totalPieceTileCount += _pieces.begin()->getNumTiles();
        }
	}

	//-------------------------------------------------------------------------------------------------
	GameState::GameState()
	{
		// initially all pieces are playable
		for (auto& remaining : m_remainingPieces)
			remaining.set();
	}

	//-------------------------------------------------------------------------------------------------
	GameState GameState::play(const Move& _move) const
	{
		GameState newGameState = *this;

		u32 turn = getPlayerTurn();
		DEBUG_ASSERT(m_remainingPieces[turn].test(_move.pieceIndex));
		newGameState.m_remainingPieces[turn].reset(_move.pieceIndex);

		Slot player = Slot(u32(Slot::P0) + turn);
		DEBUG_ASSERT(m_board.canAddPiece(player, _move.piece, { _move.position.x, _move.position.y }));
		newGameState.m_board.addPiece(player, _move.piece, _move.position);
		newGameState.m_turn = m_turn + 1;

		return newGameState;
	}

    //-------------------------------------------------------------------------------------------------
    GameState GameState::skip() const
    {
        GameState newGameState = *this;
        newGameState.m_turn = m_turn + 1;

        return newGameState;
    }

    void GameState::generateValidMoves(Slot _player, bool _ignoreCornerRule, 
                                       const Board& _board, u32 _numSlots, const Board::PositionList& _slots, const std::bitset<BlokusGame::PiecesCount>& _remainingPieces, 
                                       std::vector<Move>& _moves, std::vector<u32>* _adjacencyScores)
    {
        // reverse order to look at big pieces first (to improve sorting later)
        for (auto it = s_allPieces.rbegin(); it != s_allPieces.rend(); ++it)
        {
            u32 piece = (u32)std::distance(s_allPieces.begin(), it.base()) - 1;
            if (_remainingPieces.test(piece))
            {
                for (const Piece& p : *it)
                {
                    for (u32 i = 0; i < _numSlots; ++i)
                    {
                        std::array<ubyte2, Piece::MaxPlayableCorners> pieceMoves;
                        std::array<u32, Piece::MaxPlayableCorners> adjacencyScores;
                        u32 numMoveForPiece = _board.getPiecePlayablePositions(_player, p, _slots[i], _ignoreCornerRule, pieceMoves, _adjacencyScores ? &adjacencyScores : nullptr);

                        for (u32 j = 0; j < numMoveForPiece; ++j)
                        {
                            _moves.push_back({ p, piece, pieceMoves[j] });
                            if (_adjacencyScores)
                                _adjacencyScores->push_back(adjacencyScores[j]);
                        }
                    }
                }
            }
        }
    }

	//-------------------------------------------------------------------------------------------------
	std::vector<Move> GameState::enumerateMoves(bool _sortByHeuristic) const
	{
		std::vector<Move> moves;
		Slot playerToMove = Slot(u32(Slot::P0) + getPlayerTurn());

		Board::PositionList slots;
		u32 numSlots = m_board.computeValidSlotsForPlayer(playerToMove, slots);

		moves.reserve(m_remainingPieces[getPlayerTurn()].count() * numSlots);

        generateValidMoves(playerToMove, m_turn < 4, m_board, numSlots, slots, m_remainingPieces[getPlayerTurn()], moves);

		if (_sortByHeuristic)
		{
            std::vector<std::pair<Move, float>> moves_scores(moves.size());
            std::transform(moves.begin(), moves.end(), moves_scores.begin(),
                [&](const Move& move) -> std::pair<Move, float>
            {
                return { move, computeHeuristic(move) };
            });

			std::sort(std::begin(moves_scores), std::end(moves_scores), [this](const auto& m1, const auto& m2)
			{
				return m1.second > m2.second;
			});

            std::transform(moves_scores.begin(), moves_scores.end(), moves.begin(), [](const auto& p)
            {
                return p.first;
            });
		}
		return moves;
	}

	//-------------------------------------------------------------------------------------------------
	float GameState::computeHeuristic(const Move& _move) const
	{
		float closestDistToCenter = std::numeric_limits<float>::max();
		vec2 boardCenter = { Board::BoardSize / 2, Board::BoardSize / 2 };
		for (u32 i = 0; i < _move.piece.getNumTiles(); ++i)
		{
			vec2 tilePos = vec2{ 0.5f + (float)Piece::getTileX(_move.piece.getTile(i)), 0.5f + (float)Piece::getTileY(_move.piece.getTile(i)) };
			tilePos += vec2(_move.position.x, _move.position.y);
			closestDistToCenter = std::min(closestDistToCenter, linalg::length2(boardCenter - tilePos) / (2 * Board::BoardSize * Board::BoardSize));
		}

		DEBUG_ASSERT(closestDistToCenter <= 1);

		return _move.piece.getNumTiles() * 10.0f +
			   _move.piece.getNumCorners() +
			   1.f - closestDistToCenter;
	}

    //-------------------------------------------------------------------------------------------------
    u32 GameState::getPlayedPieceTiles(Slot _player) const
    {
        u32 result = 0;
        u32 playerIndex = u32(_player) - u32(Slot::P0);
        for (u32 i = 0; i < BlokusGame::PiecesCount; ++i)
            if (!m_remainingPieces[playerIndex].test(i))
                result += s_allPieces[i].begin()->getNumTiles();

        return result;
    }

	//-------------------------------------------------------------------------------------------------
	float GameState::computeBoardScore(Slot _player, BoardHeuristic _heuristicType) const
	{
		float score = (float)getPlayedPieceTiles(_player);

        if (_heuristicType == BoardHeuristic::RemainingTiles)
            return score;

        if (_heuristicType == BoardHeuristic::NumberOfMoves)
        {
            float numMove = float(enumerateMoves(false).size());
            score += (numMove / (numMove + 1));
        }
        else if (_heuristicType == BoardHeuristic::ReachableEmptySpace ||
                 _heuristicType == BoardHeuristic::ReachableEmptySpaceWeighted)
        {
            score += computeFreeSpaceHeuristic(_player, _heuristicType == BoardHeuristic::ReachableEmptySpaceWeighted);
        }

		return score;
	}

    //-------------------------------------------------------------------------------------------------
    struct ExpandCluster
    {
        ExpandCluster(Slot _player, const Board& _board, bool _expandInCorner)
            : m_board{ _board }, m_player{ _player }, m_expandInCorner{ _expandInCorner } {}

        const Board& m_board;
        Slot m_player;
        bool m_expandInCorner = false;
        ubyte m_clusters[Board::BoardSize][Board::BoardSize] = { {0} }; // 0 unexplored, -1 in queue, 1..n cluster index
        std::array<ubyte2, Board::BoardSize * Board::BoardSize> m_expandCluster; // queue of tiles to expand
        u32 m_expandClusterSize = 0;

        // track size of clusters
        u32 m_clusterIndex = 0;
        u16 m_clusterSize[(Board::BoardSize*Board::BoardSize)/4] = {};
        ubyte m_numPlayableSlotPerCluster[(Board::BoardSize*Board::BoardSize)/4] = {};

        void remove(u32 _expandedClusterIdx)
        {
            DEBUG_ASSERT(_expandedClusterIdx < m_expandClusterSize);
            std::swap(m_expandCluster[_expandedClusterIdx], m_expandCluster[m_expandClusterSize]);
            --m_expandClusterSize;
        }

        void addToClusterList(int x, int y)
        {
            if (x < 0 || y < 0 || x >= Board::BoardSize || y >= Board::BoardSize)
                return;

            if (m_board.getSlot(x, y) != Slot::Empty)
                return;

            if (m_board.getSlotSafe(x - 1, y) == m_player ||
                m_board.getSlotSafe(x, y - 1) == m_player || 
                m_board.getSlotSafe(x + 1, y) == m_player || 
                m_board.getSlotSafe(x, y + 1) == m_player)
                return;

            if (m_clusters[x][y] != 0)
                return;

            m_clusters[x][y] = ubyte(-1);
            m_expandCluster[m_expandClusterSize++] = { ubyte(x), ubyte(y) };
        };

        void expandTo(int _x, int _y, u32 _clusterIndex)
        {
            if (m_clusters[_x][_y] == _clusterIndex)
                return;

            DEBUG_ASSERT(m_clusters[_x][_y] == 0 || m_clusters[_x][_y] == ubyte(-1));
            m_clusters[_x][_y] = ubyte(_clusterIndex);
            m_clusterSize[_clusterIndex - 1]++;

            addToClusterList(_x - 1, _y);
            addToClusterList(_x + 1, _y);
            addToClusterList(_x, _y - 1);
            addToClusterList(_x, _y + 1);

            if (m_expandInCorner)
            {
                addToClusterList(_x - 1, _y - 1);
                addToClusterList(_x + 1, _y - 1);
                addToClusterList(_x - 1, _y + 1);
                addToClusterList(_x + 1, _y + 1);
            }
        }

        void expandFrom(ubyte2 _pos)
        {
            DEBUG_ASSERT(m_clusters[_pos.x][_pos.y] != ubyte(-1));

            if (m_clusters[_pos.x][_pos.y] != 0)
            {
                u32 clusterIdx = std::min<u32>(m_clusters[_pos.x][_pos.y], ARRAY_SIZE(m_numPlayableSlotPerCluster)) - 1;
                m_numPlayableSlotPerCluster[clusterIdx]++;
                return;
            }

            m_clusterIndex++;
            m_clusterSize[m_clusterIndex - 1] = 0;
            m_numPlayableSlotPerCluster[m_clusterIndex - 1] = 1;

            // _pos must be a valid playable slot
            expandTo(_pos.x, _pos.y, m_clusterIndex);

            while (m_expandClusterSize > 0)
            {
                ubyte2 pos = m_expandCluster[m_expandClusterSize - 1];
                remove(m_expandClusterSize - 1);

                int x = pos.x;
                int y = pos.y;
                expandTo(x, y, m_clusterIndex);
            }
        }

        void invalidateClusterPositions(const Move& _move)
        {
            auto invalidate = [this](int x, int y)
            {
                if (x >= 0 && x < Board::BoardSize && y >= 0 && y < Board::BoardSize)
                    m_clusters[x][y] = 0;
            };

            for (u32 i = 0; i < _move.piece.getNumTiles(); ++i)
            {
                int x = int(Piece::getTileX(_move.piece.getTile(i)));
                int y = int(Piece::getTileY(_move.piece.getTile(i)));
                x += int(_move.position.x);
                y += int(_move.position.y);

                invalidate(x, y);
                invalidate(x-1, y);
                invalidate(x+1, y);
                invalidate(x, y-1);
                invalidate(x, y+1);
            }
        }

        Board generateBoard(Slot _toFill) const
        {
            Board board;
            for (u32 j = 0; j < Board::BoardSize; ++j)
            {
                for (u32 i = 0; i < Board::BoardSize; ++i)
                {
                    Slot s = m_clusters[i][j] != 0 ? Slot::Empty : _toFill;
                    board.setSlot(i, j, s);
                }
            }
            return board;
        }

        void printClusters() const
        {
            for (u32 j = 0; j < Board::BoardSize; ++j)
            {
                for (u32 i = 0; i < Board::BoardSize; ++i)
                {
                    std::cout << (u32)m_clusters[i][j] << " ";
                }
                std::cout << std::endl;
            }
        }

        bool isReachable(u32 x, u32 y) const 
        {
            return m_clusters[x][y] != 0;
        }
    };

    //-------------------------------------------------------------------------------------------------
    float GameState::computeScoreUpperBound(Slot _player, BoardHeuristic) const
    {
# if 0 // too slow
        // Simulate a "game" where we add each remaining piece of a player using the best fit strategy (no other player add pieces),
        // We use the final result of the game as an upper bound of the score
        std::bitset<BlokusGame::PiecesCount> remainingPieces = m_remainingPieces[u32(_player) - u32(Slot::P0)];
        std::vector<Move> moves;
        std::vector<u32> bestFitScores;

        ExpandCluster clusterExpander(_player, getBoard(), true);
        computeReachableSlots(_player, clusterExpander);
        
        Board::PositionList positions = {};
        u32 numPosition = 0;

        auto fillListOfValidPositions = [&]()
        {
            numPosition = 0;
            for (u32 i = 0; i < Board::BoardSize; ++i)
            {
                for (u32 j = 0; j < Board::BoardSize; ++j)
                {
                    if (clusterExpander.isReachable(i, j))
                        positions[numPosition++] = { ubyte(i), ubyte(j) };
                }
            }
        };

        do
        {
            fillListOfValidPositions();

            moves.clear();
            bestFitScores.clear();
            generateValidMoves(Slot::P0, true, clusterExpander.generateBoard(Slot::P1), numPosition, positions, remainingPieces, moves, &bestFitScores);
            if (moves.size() > 0)
            {
                auto it = std::max_element(bestFitScores.begin(), bestFitScores.end());
                const Move& bestFitMove = moves[std::distance(bestFitScores.begin(), it)];
                remainingPieces.reset(bestFitMove.pieceIndex);

                clusterExpander.invalidateClusterPositions(bestFitMove);
            }
        } while (moves.size() > 0 && remainingPieces.any());

        // upperbound from unplayabe piece in remainingPieces
        float score = 0;
        for (u32 i = 0; i < BlokusGame::PiecesCount; ++i)
            if (!remainingPieces.test(i))
                score += s_allPieces[i].begin()->getNumTiles();

        return score + 1.0f;
#endif 
    }

    //-------------------------------------------------------------------------------------------------
    float GameState::computeScoreLowerBound(Slot _player, BoardHeuristic) const
    {
        return float(getPlayedPieceTiles(_player));
    }

    //-------------------------------------------------------------------------------------------------
    void GameState::computeReachableSlots(Slot _player, ExpandCluster& _expander) const
    {
        Board::PositionList slots;
        u32 numSlots = m_board.computeValidSlotsForPlayer(_player, slots);

        for (u32 i = 0; i < numSlots; ++i)
            _expander.expandFrom(slots[i]);
    }

    //-------------------------------------------------------------------------------------------------
    float GameState::computeFreeSpaceHeuristic(Slot _player, bool _weightCluster) const
    {
        ExpandCluster clusterExpander(_player , getBoard(), false);
        computeReachableSlots(_player, clusterExpander);

        float numReachables = 0;
        for (u32 i = 0; i < clusterExpander.m_clusterIndex; ++i)
        {
            if (clusterExpander.m_clusterSize[i] == 0)
                break;
            float weight = 1;
            if (_weightCluster)
                weight = float(clusterExpander.m_numPlayableSlotPerCluster[i]) / (clusterExpander.m_numPlayableSlotPerCluster[i] + 1);

            numReachables += clusterExpander.m_clusterSize[i] * weight;
        }

        return numReachables / (Board::BoardSize * Board::BoardSize);
    }

    //-------------------------------------------------------------------------------------------------
    void IAStats::start()
    {
        m_numHeuristicEvaluated = 0;
        m_numNodesExplored = 0;
        m_start = std::chrono::steady_clock::now();
    }

    void IAStats::stop()
    {
        std::chrono::duration<float> diff = std::chrono::steady_clock::now() - m_start;
        m_timeInSecond = diff.count();
    }

    float IAStats::nodePerSecond() const
    {
        return m_numNodesExplored / m_timeInSecond;
    }
}