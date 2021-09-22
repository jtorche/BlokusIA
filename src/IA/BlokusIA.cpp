#include "IA/BlokusIA.h"
#include "IA/BlokusGameHelpers.h"
#include "IA/Cache.h"
#include <numeric>

namespace BlokusIA
{
	PieceSymetries s_allPieces = {};
    u32 s_totalPieceTileCount = 0;

#ifdef _DEBUG
    thread_pool s_threadPool(1);
#else
    thread_pool s_threadPool;
#endif

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
    static Slot convertToSlot(u32 _playerIndex)
    {
        return Slot(u32(Slot::P0) + _playerIndex);
    }

	//-------------------------------------------------------------------------------------------------
	GameState::GameState()
	{
		// initially all pieces are playable
        for (auto& remaining : m_remainingPieces)
        {
            remaining.set();
            remaining.reset(BlokusGame::PiecesCount);
        }

        for (u32 i = 0; i < 4; ++i)
        {
            m_numPlaybablePos[i] = 1;
            m_playablePositions[i][0] = Board::getStartingPosition(convertToSlot(i));
        }
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

		Slot player = convertToSlot(turn);
		DEBUG_ASSERT(m_board.canAddPiece(player, _move.piece, { _move.position.x, _move.position.y }));
		newGameState.m_board.addPiece(player, _move.piece, _move.position);
        newGameState.m_playedTiles[turn] += _move.piece.getNumTiles();
        newGameState.updatePlayablePositions(player, _move);

        constexpr u32 pieceSpaceCompensation[5] = { 4,6,8,10,12 };
        DEBUG_ASSERT(_move.piece.getNumTiles() <= 5);
        newGameState.m_pieceSpaceScoreCompensation[turn] += pieceSpaceCompensation[_move.piece.getNumTiles() - 1];

		newGameState.m_turn = m_turn + 1;

		return newGameState;
	}

    //-------------------------------------------------------------------------------------------------
    GameState GameState::skip() const
    {
        GameState newGameState = *this;
        newGameState.m_remainingPieces[getPlayerTurn()].set(BlokusGame::PiecesCount);
        newGameState.m_turn = m_turn + 1;

        return newGameState;
    }

	//-------------------------------------------------------------------------------------------------
	std::vector<Move> GameState::enumerateMoves() const
	{
        if (m_remainingPieces[getPlayerTurn()].test(BlokusGame::PiecesCount))
            return {};

		std::vector<Move> moves;
		Slot playerToMove = convertToSlot(getPlayerTurn());

		const Board::PlayableSlots& slots = m_playablePositions[getPlayerTurn()];
        u32 numSlots = m_numPlaybablePos[getPlayerTurn()];

		moves.reserve(512);

		// reverse order to look at big pieces first (to improve sorting later)
		for (auto it = s_allPieces.rbegin() ; it != s_allPieces.rend() ; ++it)
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

		return moves;
	}

    //-------------------------------------------------------------------------------------------------
    void GameState::findCandidatMoves(MoveHeuristic _heuristic, u32 _numMoves, std::vector<Move>& _allMoves) const
    {
        _numMoves = std::min(_numMoves, u32(_allMoves.size()));

        std::vector<std::pair<Move, float>> moves_scores(_allMoves.size());
        std::transform(_allMoves.begin(), _allMoves.end(), moves_scores.begin(),
            [&](const Move& move) -> std::pair<Move, float>
        {
            return { move, computeHeuristic(move, _heuristic) };
        });
    
    	std::partial_sort(std::begin(moves_scores), std::begin(moves_scores) + _numMoves, std::end(moves_scores),
            [this](const auto& m1, const auto& m2)
    	{
    		return m1.second > m2.second;
    	});
    
        _allMoves.resize(_numMoves);
        std::transform(moves_scores.begin(), moves_scores.begin() + _numMoves, _allMoves.begin(), [](const auto& p)
        {
            return p.first;
        });

        if (getTurnCount() < g_NumTurnToRushCenter * 4 && !moves_scores.empty())
        {
            // only keep best move to rush center
            auto it = std::find_if(moves_scores.begin(), moves_scores.end(), [&](const auto& val)
            {
                return val.second != moves_scores.begin()->second;
            });

            if (it != moves_scores.end())
                _allMoves.resize(std::distance(moves_scores.begin(), it));
        }
    }

    //-------------------------------------------------------------------------------------------------
    u32 GameState::getBestMoveIndex(const std::vector<float>& _scores)
    {
        auto best = std::max_element(_scores.begin(), _scores.end());
        size_t numBestScore = std::count_if(_scores.begin(), _scores.end(), [bestScore = *best](float val) { return val == bestScore; });

        u32 selectedMove = rand() % numBestScore;
        u32 counter = 0;
        for (u32 i = 0; i < _scores.size(); ++i)
        {
            if (_scores[i] == *best)
            {
                if (selectedMove == counter++)
                    return i;
            }
        }

        DEBUG_ASSERT(false);
        return 0;
    }

	//-------------------------------------------------------------------------------------------------
	float GameState::computeHeuristic(const Move& _move, MoveHeuristic _moveHeuristic) const
	{
        float distToCenterHeuristic = 0;

        // first 4 rounds, we favor a rush to the center
        if (getTurnCount() < g_NumTurnToRushCenter*4)
        {
            float closestDistToCenter = std::numeric_limits<float>::max();
            vec2 boardCenter = { Board::BoardSize / 2, Board::BoardSize / 2 };
            for (u32 i = 0; i < _move.piece.getNumTiles(); ++i)
            {
                vec2 tilePos = vec2{ 0.5f + (float)Piece::getTileX(_move.piece.getTile(i)), 0.5f + (float)Piece::getTileY(_move.piece.getTile(i)) };
                tilePos += vec2(_move.position.x, _move.position.y);
                closestDistToCenter = std::min(closestDistToCenter, linalg::length2(boardCenter - tilePos) / (2 * Board::BoardSize * Board::BoardSize));
            }
            distToCenterHeuristic = 1.f - closestDistToCenter;

            return distToCenterHeuristic;
        }

		DEBUG_ASSERT(distToCenterHeuristic <= 1);

        if(_moveHeuristic == MoveHeuristic::TileCount)
		    return _move.piece.getNumTiles() + distToCenterHeuristic;
        else if (_moveHeuristic == MoveHeuristic::ReachableSpace)
        {
            Slot playerToMove = convertToSlot(getPlayerTurn());
            GameState moveState = play(_move);
            float numReachable = moveState.computeFreeSpaceHeuristic(playerToMove, 0, true);
            numReachable += moveState.getPlayedPieceTiles(playerToMove);

            // we still favor big piece
            return numReachable * 10 + float(_move.piece.getNumTiles()) + distToCenterHeuristic;
        }

        DEBUG_ASSERT(0);
        return 0;
	}

    //-------------------------------------------------------------------------------------------------
    u32 GameState::getPlayedPieceTiles(Slot _player) const
    {
        return m_playedTiles[u32(_player) - u32(Slot::P0)];
    }

	//-------------------------------------------------------------------------------------------------
	float GameState::computeBoardScore(Slot _player, BoardHeuristic _heuristicType) const
	{
        return g_cache.computeBoardScore(*this, _player, _heuristicType);
	}

    //-------------------------------------------------------------------------------------------------
    float GameState::computeBoardScoreInner(Slot _player, BoardHeuristic _heuristicType) const
    {
        float negativeScore = 0; // second place means score = -1e6, third = -2e6, ...

        // Detect if the player can't win anymore
        if (noMoveLeft(_player))
        {
            for (u32 i = 0; i < 4; ++i)
            {
                if(convertToSlot(i) != _player && getPlayedPieceTiles(_player) < getPlayedPieceTiles(convertToSlot(i)))
                    negativeScore -= 1000000.f;
            }
        }

        // we can't win anymore, so we return a negative heuristic to avoid this scenario at all cost
        if (negativeScore < 0)
            return negativeScore;

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
                // We sum the m_numBorderTileCache to make sure playing big pieces is better than playing small pieces
                // Because playing big pieces also loose some free space
                float freeSpaceHeuristic = computeFreeSpaceHeuristic(_player, powFactor, false);
                return freeSpaceHeuristic + (float)(getPlayedPieceTiles(_player) + m_pieceSpaceScoreCompensation[u32(_player) - u32(Slot::P0)]);
            }
            else
            {
                // Here big pieces are always prioritized
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

        template<int OffsetX, int OffsetY>
        void addToClusterList(int x, int y)
        {
            static_assert(!(OffsetX != 0 && OffsetY != 0));
            if constexpr (OffsetX < 0)
            {
                if (x + OffsetX < 0)
                    return;
            }
            else if constexpr (OffsetX > 0)
            {
                if (x + OffsetX >= Board::BoardSize)
                    return;
            }
            else if constexpr (OffsetY < 0)
            {
                if (y + OffsetY < 0)
                    return;
            }
            else if constexpr (OffsetY > 0)
            {
                if (y + OffsetY >= Board::BoardSize)
                    return;
            }

            x += OffsetX;
            y += OffsetY;
            
            if (m_clusters[x][y] != 0)
                return;

            if (m_state.getBoard().getSlot(x, y) != Slot::Empty)
                return;

            if (!m_includeSideUnreachableEmptySlot)
            {
                if (m_state.getBoard().getSlotSafe<-1,0>(x, y) == m_player ||
                    m_state.getBoard().getSlotSafe<1,0>(x, y) == m_player ||
                    m_state.getBoard().getSlotSafe<0,-1>(x, y) == m_player ||
                    m_state.getBoard().getSlotSafe<0,1>(x, y) == m_player)
                    return;
            }

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

            addToClusterList<-1,0>(_x, _y);
            addToClusterList<1,0>(_x, _y);
            addToClusterList<0,-1>(_x, _y);
            addToClusterList<0,1>(_x, _y);
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
        u32 playerIndex = u32(_player) - u32(Slot::P0);
        const Board::PlayableSlots& slots = m_playablePositions[playerIndex];
        u32 numSlots = m_numPlaybablePos[playerIndex];

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
    void GameState::updatePlayablePositions(Slot _player, const Move& _move)
    {
        for (u32 p = 0; p < 4; ++p)
        {
            for (u32 i = 0; i < m_numPlaybablePos[p]; ++i)
            {
                if (getTurnCount() < p) // the round of move, we skip this process because the first playable slot is an exception
                    continue;

                if (!m_board.isValidPlayableSlot(convertToSlot(p), m_playablePositions[p][i]))
                {
                    std::swap(m_playablePositions[p][i], m_playablePositions[p][m_numPlaybablePos[p]-1]);
                    m_numPlaybablePos[p]--;
                    i--;
                }
            }
        }

        ivec2 iBoardPos = { int(_move.position.x), int(_move.position.y) };
        u32 playerIndex = u32(_player) - u32(Slot::P0);
        for (u32 i = 0; i < Piece::MaxTile; ++i)
        {
            if (_move.piece.getTile(i) == 0)
                break;

            ivec2 tilePos = { int(Piece::getTileX(_move.piece.getTile(i))), int(Piece::getTileY(_move.piece.getTile(i))) };
            tilePos += iBoardPos;

            ubyte corners[4];
            _move.piece.getCorners(i, corners);

            for (u32 c = 0; c < 4; ++c)
            {
                if (corners[c])
                {
                    ivec2 cornerPos = tilePos;
                    switch (c)
                    {
                    case 0:
                        cornerPos += ivec2(-1, -1); break;
                    case 1:
                        cornerPos += ivec2(1, -1); break;
                    case 2:
                        cornerPos += ivec2(1, 1); break;
                    case 3:
                        cornerPos += ivec2(-1, 1); break;
                    }

                    if (cornerPos.x >= 0 && cornerPos.x < Board::BoardSize && cornerPos.y >= 0 && cornerPos.y < Board::BoardSize)
                    {
                        ubyte2 pos = { ubyte(cornerPos.x), ubyte(cornerPos.y) };
                        if (m_board.isValidPlayableSlot(_player, pos))
                        {
                            DEBUG_ASSERT(m_numPlaybablePos[playerIndex] < Board::MaxPlayableCorners);
                            m_playablePositions[playerIndex][m_numPlaybablePos[playerIndex]++] = pos;
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------------------------------------
    u32 BaseIA::maxMoveToLookAt(const GameState&) const
    {
        return 32;
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