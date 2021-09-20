#include "IA/BlokusIA.h"
#include "IA/BlokusGameHelpers.h"
#include "IA/Cache.h"
#include <numeric>

namespace BlokusIA
{
	PieceSymetries s_allPieces = {};
    u32 s_totalPieceTileCount = 0;
    thread_pool s_threadPool;
    GameStateCache g_cache;

	//-------------------------------------------------------------------------------------------------
	void initBlokusIA()
	{
		s_allPieces = Helpers::getAllPieceSymetries();
        s_totalPieceTileCount = 0;
        for (const auto& _pieces : s_allPieces)
        {
            s_totalPieceTileCount += _pieces.begin()->getNumTiles();
        }
        srand((u32)time(nullptr));
	}

    //-------------------------------------------------------------------------------------------------
    GameStateCache& getGlobalCache()
    {
        return g_cache;
    }

	//-------------------------------------------------------------------------------------------------
	GameState::GameState()
	{
		// initially all pieces are playable
		for (auto& remaining : m_remainingPieces)
			remaining.set();
	}

    //-------------------------------------------------------------------------------------------------
    bool GameState::operator==(const GameState& _state) const
    {
        return m_board == _state.m_board &&
               m_remainingPieces[0] == _state.m_remainingPieces[0] &&
               m_remainingPieces[1] == _state.m_remainingPieces[1] &&
               m_remainingPieces[2] == _state.m_remainingPieces[2] &&
               m_remainingPieces[3] == _state.m_remainingPieces[3] &&
               m_turn == _state.m_turn;
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

	//-------------------------------------------------------------------------------------------------
	std::vector<Move> GameState::enumerateMoves(bool _sortByHeuristic) const
	{
		std::vector<Move> moves;
		Slot playerToMove = Slot(u32(Slot::P0) + getPlayerTurn());

		Board::PlayableSlots slots;
		u32 numSlots = m_board.computeValidSlotsForPlayer(playerToMove, slots);

		moves.reserve(m_remainingPieces[getPlayerTurn()].count() * numSlots);

		// reverse order to look at big pieces first (to improve sorting later)
		for (auto it = s_allPieces.rbegin() ;  it != s_allPieces.rend() ; ++it)
		{
			u32 piece = (u32)std::distance(s_allPieces.begin(), it.base()) - 1;
			if (m_remainingPieces[getPlayerTurn()].test(piece))
			{
				for (const Piece& p : *it)
				{
					for (u32 i = 0; i < numSlots; ++i)
					{
						std::array<ubyte2, Piece::MaxPlayableCorners> pieceMoves;
						u32 numMoveForPiece = m_board.getPiecePlayablePositions(playerToMove, p, slots[i], pieceMoves, m_turn < 4);

						for (u32 j = 0; j < numMoveForPiece; ++j)
						{
							moves.push_back({ p, piece, pieceMoves[j] });
						}
					}
				}
			}
		}

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

		return _move.piece.getNumTiles() +
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
        return g_cache.computeBoardScore(*this, _player, _heuristicType);
	}

    //-------------------------------------------------------------------------------------------------
    float GameState::computeBoardScoreInner(Slot _player, BoardHeuristic _heuristicType) const
    {
        if (_heuristicType == BoardHeuristic::RemainingTiles)
            return (float)getPlayedPieceTiles(_player);

        //if (_heuristicType == BoardHeuristic::ReachableEmptySpace ||
        //    _heuristicType == BoardHeuristic::ReachableEmptySpaceWeighted || 
        //    _heuristicType == BoardHeuristic::ReachableEmptySpaceWeighted2 || 
        //    _heuristicType == BoardHeuristic::ReachableEmptySpaceOnly ||
        //    _heuristicType == BoardHeuristic::ReachableEmptySpaceWeightedOnly)
        {
            float powFactor = 0;
            switch (_heuristicType)
            {
            case BoardHeuristic::ReachableEmptySpaceWeighted:
            case BoardHeuristic::ReachableEmptySpaceWeightedOnly:
                powFactor = 1; break;
            case BoardHeuristic::ReachableEmptySpaceWeighted2:
                powFactor = 2; break;
            }

            if (_heuristicType == BoardHeuristic::ReachableEmptySpaceOnly ||
                _heuristicType == BoardHeuristic::ReachableEmptySpaceWeightedOnly)
            {
                // we include unreachable empty side slot so we can sum with "getPlayedPieceTiles" without any bias
                float freeSpaceHeuristic = computeFreeSpaceHeuristic(_player, powFactor, true);
                return freeSpaceHeuristic + (float)getPlayedPieceTiles(_player);
            }
            else
            {
                float freeSpaceHeuristic = computeFreeSpaceHeuristic(_player, powFactor, false);
                return (float)getPlayedPieceTiles(_player) + freeSpaceHeuristic / (Board::BoardSize * Board::BoardSize);
            }
        }
    }

    //-------------------------------------------------------------------------------------------------
    struct ExpandCluster
    {
        ExpandCluster(Slot _player, const GameState& _state, bool _includeSideUnreachableEmptySlot)
            : m_state{ _state }, m_player{ _player }, m_includeSideUnreachableEmptySlot{ _includeSideUnreachableEmptySlot } {}

        const GameState& m_state;
        Slot m_player;
        bool m_includeSideUnreachableEmptySlot = false;
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

            if (m_state.getBoard().getSlot(x, y) != Slot::Empty)
                return;

            if (!m_includeSideUnreachableEmptySlot)
            {
                if (m_state.getBoard().getSlotSafe(x - 1, y) == m_player ||
                    m_state.getBoard().getSlotSafe(x, y - 1) == m_player ||
                    m_state.getBoard().getSlotSafe(x + 1, y) == m_player ||
                    m_state.getBoard().getSlotSafe(x, y + 1) == m_player)
                    return;
            }

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

            //if (m_expandInCorner)
            //{
            //    addToClusterList(_x - 1, _y - 1);
            //    addToClusterList(_x + 1, _y - 1);
            //    addToClusterList(_x - 1, _y + 1);
            //    addToClusterList(_x + 1, _y + 1);
            //}
        }

        void expandFrom(ubyte2 _pos)
        {
            DEBUG_ASSERT(m_clusters[_pos.x][_pos.y] != ubyte(-1));

            if (m_clusters[_pos.x][_pos.y] != 0)
            {
                m_numPlayableSlotPerCluster[m_clusters[_pos.x][_pos.y] - 1]++;
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
    };

    //-------------------------------------------------------------------------------------------------
    void GameState::computeReachableSlots(Slot _player, ExpandCluster& _expander) const
    {
        Board::PlayableSlots slots;
        u32 numSlots = m_board.computeValidSlotsForPlayer(_player, slots);

        for (u32 i = 0; i < numSlots; ++i)
            _expander.expandFrom(slots[i]);
    }

    //-------------------------------------------------------------------------------------------------
    float GameState::computeFreeSpaceHeuristic(Slot _player, float _weightCluster, bool _includeSideUnreachableEmptySlot) const
    {
        ExpandCluster clusterExpander(_player , *this, _includeSideUnreachableEmptySlot);
        computeReachableSlots(_player, clusterExpander);

        float numReachables = 0;
        for (u32 i = 0; i < clusterExpander.m_clusterIndex; ++i)
        {
            if (clusterExpander.m_clusterSize[i] == 0)
                break;

            float weight = powf(float(clusterExpander.m_numPlayableSlotPerCluster[i]) / (clusterExpander.m_numPlayableSlotPerCluster[i] + 1), _weightCluster);
            numReachables += clusterExpander.m_clusterSize[i] * weight;
        }

        return numReachables;
    }

    //-------------------------------------------------------------------------------------------------
    size_t BaseIA::maxMoveToLookAt(const GameState& _state) const
    {
        &_state;
        return 2000;
        //if (_state.getTurnCount() < 12)
        //    return 4;
        //else
        //    return 16;
    }

    //-------------------------------------------------------------------------------------------------
    void BaseIA::start()
    {
        m_numHeuristicEvaluated = 0;
        m_numNodesExplored = 0;
        m_start = std::chrono::steady_clock::now();
    }

    void BaseIA::stop()
    {
        std::chrono::duration<float> diff = std::chrono::steady_clock::now() - m_start;
        m_timeInSecond = diff.count();
    }

    float BaseIA::nodePerSecond() const
    {
        return m_numNodesExplored / m_timeInSecond;
    }

    u32 BaseIA::getNumNodeExplored() const
    {
        return m_numNodesExplored;
    }
}