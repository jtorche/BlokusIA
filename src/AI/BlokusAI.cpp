#include <numeric>
#include <limits>

#include "AI/BlokusAI.h"
#include "AI/BlokusGameHelpers.h"
#include "AI/Cache.h"

namespace blokusAI
{
	PieceSymetries s_allPieces = {};
    u32 s_totalPieceTileCount = 0;
    std::default_random_engine s_rand;

#ifdef _DEBUG
    thread_pool s_threadPool(1);
#else
    thread_pool s_threadPool;
#endif

    GameStateCache g_cache;

    const float GameState::s_endGameScore = std::numeric_limits<float>::max();

	//-------------------------------------------------------------------------------------------------
	void initBlokusAI()
	{
		s_allPieces = Helpers::getAllPieceSymetries();
        s_totalPieceTileCount = 0;
        for (const auto& _pieces : s_allPieces)
        {
            s_totalPieceTileCount += _pieces.begin()->getNumTiles();
        }

        u32 seed = (u32)time(nullptr);
        std::cout << "Seed:" << seed << std::endl;
        s_rand = std::default_random_engine(seed);
	}

    //-------------------------------------------------------------------------------------------------
    void printAllPieces()
    {
        u32 pieceIndex = 1;
        for (const auto& _pieces : s_allPieces)
        {
            std::cout << "------------------------------\n";
            std::cout << "Piece " << pieceIndex++ << ":\n";
            for (const auto& piece : _pieces)
            {
                std::cout << "--------\n";
                piece.print();
            }
        }
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

    static u32 convertToIndex(Slot _player)
    {
        return u32(_player) - u32(Slot::P0);
    }

    //-------------------------------------------------------------------------------------------------
    struct ExpandCluster
    {
        ExpandCluster(Slot _player, ReachableSlots& _reachableSlots, const Board& _board)
            : m_board{ _board }, m_reachableSlots{ _reachableSlots },
            m_player{ _player }, m_playerIndex{ ubyte(convertToIndex(_player)) } {}

        const Board& m_board;
        ReachableSlots& m_reachableSlots;
        Slot m_player;
        ubyte m_playerIndex;
        std::array<ubyte2, Board::BoardSize * Board::BoardSize> m_expandCluster; // queue of tiles to expand
        u32 m_expandClusterSize = 0;

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

            if (m_reachableSlots.m_clusters[x][y] != 0)
                return;

            if (m_board.getSlot(x, y) != Slot::Empty)
                return;

            if (m_board.getContactRuleCache(x, y) & (ubyte(1) << m_playerIndex))
                return;

            m_reachableSlots.m_clusters[x][y] = ubyte(-1);
            m_expandCluster[m_expandClusterSize++] = { ubyte(x), ubyte(y) };
        };

        void expandTo(int _x, int _y, u32 _clusterIndex)
        {
            if (m_reachableSlots.m_clusters[_x][_y] == _clusterIndex)
                return;

            DEBUG_ASSERT(m_reachableSlots.m_clusters[_x][_y] == 0 || m_reachableSlots.m_clusters[_x][_y] == ubyte(-1));
            m_reachableSlots.m_clusters[_x][_y] = ubyte(_clusterIndex);
            m_reachableSlots.m_clusterSize[_clusterIndex - 1]++;

            addToClusterList<-1, 0>(_x, _y);
            addToClusterList<1, 0>(_x, _y);
            addToClusterList<0, -1>(_x, _y);
            addToClusterList<0, 1>(_x, _y);
        }

        void expandFrom(ubyte2 _pos)
        {
            DEBUG_ASSERT(m_reachableSlots.m_clusters[_pos.x][_pos.y] != ubyte(-1));

            if (m_reachableSlots.m_clusters[_pos.x][_pos.y] != 0)
            {
                m_reachableSlots.m_numPlayableSlotsPerCluster[m_reachableSlots.m_clusters[_pos.x][_pos.y] - 1]++;
                return;
            }

            m_reachableSlots.m_numClusters++;
            m_reachableSlots.m_clusterSize[m_reachableSlots.m_numClusters - 1] = 0;
            m_reachableSlots.m_numPlayableSlotsPerCluster[m_reachableSlots.m_numClusters - 1] = 1;

            // _pos must be a valid playable slot
            expandTo(_pos.x, _pos.y, m_reachableSlots.m_numClusters);

            while (m_expandClusterSize > 0)
            {
                ubyte2 pos = m_expandCluster[m_expandClusterSize - 1];
                remove(m_expandClusterSize - 1);

                int x = pos.x;
                int y = pos.y;
                expandTo(x, y, m_reachableSlots.m_numClusters);
            }
        }

        void print() const
        {
            for (u32 j = 0; j < Board::BoardSize; ++j)
            {
                for (u32 i = 0; i < Board::BoardSize; ++i)
                {
                    std::cout << u32(m_reachableSlots.m_clusters[i][j]);
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    };

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
            m_numPlayablePos[i] = 1;
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

        for (u32 i = 0; i < 4; ++i)
            computeReachableSlots(newGameState.m_reachableSlotsCache[i], convertToSlot(i), newGameState.getBoard(), m_playablePositions[i], m_numPlayablePos[i]);

        // find if no move left
        for (u32 i = 0; i < 4; ++i)
        {
            bool hasAnyMove = false;
            newGameState.visitMoves(i, [&](auto&&...)
            {
                hasAnyMove = true;
                return false;
            });
            if (!hasAnyMove)
                newGameState.m_remainingPieces[i].set(BlokusGame::PiecesCount);
        }

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
	std::vector<std::pair<Move, float>> GameState::enumerateMoves(MoveHeuristic _moveHeuristic, CustomHeuristicInterface* _customHeuristic) const
	{
		std::vector<std::pair<Move, float>> moves;
        moves.reserve(512);

        visitMoves(getPlayerTurn(), [&](Move _move, ubyte2 _playablePos) 
        { 
            float heuristic = 0;
            if (_moveHeuristic == MoveHeuristic::Custom)
                heuristic = _customHeuristic->moveHeuristic(*this, _move, _playablePos);
            else
                heuristic = computeHeuristic(_move, _playablePos, _moveHeuristic);

            moves.push_back({ _move, heuristic });
            return true; 
        });

		return moves;
	}

    //-------------------------------------------------------------------------------------------------
    void GameState::findCandidatMoves(u32 _numMoves, std::vector<std::pair<Move, float>>& _allMoves, u32 _numTurnToForceBestMoveHeuisitc) const
    {
        if (_allMoves.empty())
            return;

        _numMoves = std::min(_numMoves, u32(_allMoves.size()));
    
    	std::partial_sort(std::begin(_allMoves), std::begin(_allMoves) + _numMoves, std::end(_allMoves),
            [this](const auto& m1, const auto& m2)
    	{
    		return m1.second > m2.second;
    	});
    
        _allMoves.resize(_numMoves);

        if (getTurnCount() < _numTurnToForceBestMoveHeuisitc * 4 && !_allMoves.empty())
        {
            // only keep best move to rush center
            auto it = std::find_if(_allMoves.begin(), _allMoves.end(), [&](const auto& val)
            {
                return val.second != _allMoves.begin()->second;
            });

            if (it != _allMoves.end())
                _allMoves.resize(std::distance(_allMoves.begin(), it));
        }
    }

    std::vector<std::pair<Move, float>> GameState::findMovesToLookAt(MoveHeuristic _moveHeuristic, u32 _maxMoveToLookAt, u32 _numTurnToForceBestHeuristic, 
                                                                     const MultiSourceMoveHeuristicParam* _multiSrcParam, CustomHeuristicInterface* _customHeuristic) const
    {
        MoveHeuristic heuristic = _moveHeuristic == MoveHeuristic::MultiSource_Custom || _moveHeuristic == MoveHeuristic::MultiSource ? MoveHeuristic::TileCount : _moveHeuristic;

        auto moves = enumerateMoves(heuristic, _customHeuristic);
        if (moves.empty())
            moves = enumerateMoves(MoveHeuristic::TileCount);

        if (moves.empty())
            return {};

        if (_moveHeuristic == MoveHeuristic::MultiSource_Custom || _moveHeuristic == MoveHeuristic::MultiSource)
        {
            auto curBegin = moves.begin();

            // Find move according to the bridge heuristic
            std::for_each(curBegin, moves.end(), [&](auto& move_score)
            {
                auto& [move, score] = move_score;
                score = getBoard().isPieceConnectedToBridge(convertToSlot(getPlayerTurn()), move.piece, move.position) ? computeHeuristic(move, {}, MoveHeuristic::TileCount_DistCenter) : 0.f;
            });

            u32 numBridgeMove = std::min<u32>(_multiSrcParam->m_numPiecesWithBridge, u32(std::distance(curBegin, moves.end())));
            std::partial_sort(curBegin, curBegin + numBridgeMove, std::end(moves),
                              [](const auto& m1, const auto& m2) { return m1.second > m2.second; });

            // Find move according to the bridge heuristic
            curBegin = std::remove_if(curBegin, curBegin + numBridgeMove, [&](auto& move_score) { return move_score.second == 0; });

            // Find move according to the dist center
            {
                std::for_each(curBegin, moves.end(), [&](auto& move_score)
                {
                    auto& [move, score] = move_score;
                    score = computeHeuristic(move, {}, MoveHeuristic::TileCount_DistCenter);
                });

                u32 numMoves = std::min<u32>(_multiSrcParam->m_numPieceAtCenter, u32(std::distance(curBegin, moves.end())));
                std::partial_sort(curBegin, curBegin + numMoves, std::end(moves),
                                  [](const auto& m1, const auto& m2) { return m1.second > m2.second; });

                curBegin += numMoves;
            }

            // Find move according to the tile count heuristic (to fill remaining spot)
            {
                std::for_each(curBegin, moves.end(), [&](auto& move_score)
                {
                    auto& [move, score] = move_score;
                    score = computeHeuristic(move, {}, MoveHeuristic::TileCount);
                });

                u32 totalMovesFromMultiSrc = _multiSrcParam->m_numPieceAtCenter + _multiSrcParam->m_numPiecesWithBridge;
                const u32 remainingMoveToFind = u32(std::distance(moves.begin(), curBegin)) < totalMovesFromMultiSrc ? totalMovesFromMultiSrc - u32(std::distance(moves.begin(), curBegin)) : 0;
                u32 numMoves = std::min<u32>(remainingMoveToFind, u32(std::distance(curBegin, moves.end())));
                std::partial_sort(curBegin, curBegin + numMoves, std::end(moves),
                    [](const auto& m1, const auto& m2) { return m1.second > m2.second; });

                curBegin += numMoves;
            }

            moves.resize(std::distance(moves.begin(), curBegin));
            if (_moveHeuristic == MoveHeuristic::MultiSource_Custom)
            {
                std::for_each(moves.begin(), moves.end(), [&](auto& move_score)
                {
                    auto& [move, score] = move_score;
                    score = _customHeuristic->moveHeuristic(*this, move, {});
                });

                std::stable_sort(std::begin(moves), std::end(moves), [](const auto& m1, const auto& m2) { return m1.second > m2.second; });
            }

            if (moves.size() > _maxMoveToLookAt)
                moves.resize(_maxMoveToLookAt);
        }
        else
        {
            findCandidatMoves(_maxMoveToLookAt, moves, _numTurnToForceBestHeuristic);
        }

        return moves;
    }

    //-------------------------------------------------------------------------------------------------
    static thread_local std::vector<std::vector<float>::const_iterator> g_sortedScoreIterator;
    u32 GameState::getBestMoveIndex(const std::vector<float>& _scores, u32 _amongNBestMoves)
    {
        g_sortedScoreIterator.resize(0);

        for (auto it = _scores.begin(); it != _scores.end(); ++it)
            g_sortedScoreIterator.push_back(it);

        auto best = std::max_element(_scores.begin(), _scores.end());

        // If the ia detect a win, force it
        if (*best > s_endGameScore * 0.5)
            _amongNBestMoves = 1;

        size_t numBestScores = std::count_if(_scores.begin(), _scores.end(), [bestScore = *best](float val) { return val == bestScore; });
        numBestScores = std::max<size_t>(numBestScores, std::min<size_t>(_amongNBestMoves, _scores.size()));
        std::partial_sort(std::begin(g_sortedScoreIterator), std::begin(g_sortedScoreIterator) + numBestScores, std::end(g_sortedScoreIterator),
                          [](auto it1, auto it2) { return *it1 > *it2; });

        u32 selectedMove = s_rand() % numBestScores;
        return (u32)std::distance(_scores.begin(), g_sortedScoreIterator[selectedMove]);
    }

	//-------------------------------------------------------------------------------------------------
	float GameState::computeHeuristic(const Move& _move, ubyte2 _playablePos, MoveHeuristic _moveHeuristic) const
	{
        if (_moveHeuristic == MoveHeuristic::TileCount)
            return _move.piece.getNumTiles();

        Slot playerToMove = convertToSlot(getPlayerTurn());

        float distToCenterHeuristic = 0;
        {
            float closestDistToCenter = std::numeric_limits<float>::max();
            vec2 boardCenter = { Board::BoardSize / 2, Board::BoardSize / 2 };
            for (u32 i = 0; i < _move.piece.getNumTiles(); ++i)
            {
                vec2 tilePos = vec2{ 0.5f + (float)Piece::getTileX(_move.piece.getTile(i)), 0.5f + (float)Piece::getTileY(_move.piece.getTile(i)) };
                tilePos += vec2(_move.position.x, _move.position.y);
                closestDistToCenter = std::min(closestDistToCenter, linalg::length2(boardCenter - tilePos) / (2 * Board::BoardSize * Board::BoardSize));
            }
            distToCenterHeuristic = 1.f - closestDistToCenter; // [0 - 1]
            DEBUG_ASSERT(distToCenterHeuristic <= 1);
        }

        if(_moveHeuristic == MoveHeuristic::TileCount_DistCenter)
		    return _move.piece.getNumTiles() + distToCenterHeuristic;
        else if (_moveHeuristic == MoveHeuristic::WeightedReachableSpace || 
                 _moveHeuristic == MoveHeuristic::ExtendingReachableSpace)
        {
            ubyte clusterIndex = m_reachableSlotsCache[getPlayerTurn()].m_clusters[_playablePos.x][_playablePos.y];
            u32 clusterCategory = std::min<u32>(m_reachableSlotsCache[getPlayerTurn()].m_numPlayableSlotsPerCluster[clusterIndex - 1], 4);
            u32 factor = 1;
            switch (clusterCategory)
            {
            case 1:
                factor = 8; break;
            case 2:
                factor = 5; break;
            case 3:
                factor = 4; break;
            default:
            case 4:
                factor = 1; break;
            }

            // Max is 8*BoardSize*BoardSize
            float weightedReachableSpace = float(factor) * m_reachableSlotsCache[getPlayerTurn()].m_clusterSize[clusterIndex - 1];
            weightedReachableSpace /= (8 * Board::BoardSize * Board::BoardSize);
            weightedReachableSpace = 1.f + weightedReachableSpace * 9; // [1 - 10]

            if (_moveHeuristic == MoveHeuristic::ExtendingReachableSpace && getTurnCount() >= 12) // hardcoded 3 turn per player to only rush center
            {
                GameState nextState = play(_move);
                // Max is BoardSize*BoardSize
                float extendingReachableSpace = nextState.computeFreeSpaceHeuristic(playerToMove, 0) - computeFreeSpaceHeuristic(playerToMove, 0);
                extendingReachableSpace = std::max(0.f, extendingReachableSpace);
                extendingReachableSpace /= (Board::BoardSize * Board::BoardSize);
                extendingReachableSpace = 10.f + extendingReachableSpace * 90; // [10 - 100]
                return extendingReachableSpace + weightedReachableSpace + distToCenterHeuristic; // [11 - 111]
            }
            else
                return weightedReachableSpace + distToCenterHeuristic;
        }

        DEBUG_ASSERT(0);
        return 0;
	}

    //-------------------------------------------------------------------------------------------------
    u32 GameState::getPlayedPieceTiles(Slot _player) const
    {
        return m_playedTiles[convertToIndex(_player)];
    }

	//-------------------------------------------------------------------------------------------------
	float GameState::computeBoardScore(Slot _player, BoardHeuristic _heuristicType, CustomHeuristicInterface* _customHeuristic) const
	{
        return computeBoardScoreInner(_player, _heuristicType, _customHeuristic);
	}

    //-------------------------------------------------------------------------------------------------
    float GameState::computeBoardScoreInner(Slot _player, BoardHeuristic _heuristicType, CustomHeuristicInterface * _customHeuristic) const
    {
        if (_heuristicType == BoardHeuristic::Custom)
            return _customHeuristic->boardHeuristic(*this, _player);

        u32 numOpponentsDefeated = 0;
        for (u32 i = 0; i < 4; ++i)
        {
            if (convertToSlot(i) != _player)
            {
                if (noMoveLeft(convertToSlot(i)) && getPlayedPieceTiles(_player) >= getPlayedPieceTiles(convertToSlot(i)))
                    numOpponentsDefeated++;
            }
        }

        if (numOpponentsDefeated == 3)
            return s_endGameScore;

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
            case BoardHeuristic::ReachableEmptySpaceWeighted2Only:
                powFactor = 2; break;
            }

            if (_heuristicType == BoardHeuristic::ReachableEmptySpaceOnly ||
                _heuristicType == BoardHeuristic::ReachableEmptySpaceWeightedOnly ||
                _heuristicType == BoardHeuristic::ReachableEmptySpaceWeighted2Only)
            {
                // We sum the m_numBorderTileCache to make sure playing big pieces is better than playing small pieces
                // Because playing big pieces also loose some free space
                float freeSpaceHeuristic = computeFreeSpaceHeuristic(_player, powFactor);
                return freeSpaceHeuristic + (float)(getPlayedPieceTiles(_player) + m_pieceSpaceScoreCompensation[convertToIndex(_player)]);
            }
            else
            {
                // Here big pieces are always prioritized
                float freeSpaceHeuristic = computeFreeSpaceHeuristic(_player, powFactor);
                return (float)getPlayedPieceTiles(_player) + freeSpaceHeuristic / (Board::BoardSize * Board::BoardSize);
            }
        }
    }

    //-------------------------------------------------------------------------------------------------
    void GameState::computeReachableSlots(ReachableSlots& _result, Slot _player, const Board& _board, const Board::PlayableSlots& _playableSlots, u32 _numPlayableSlots)
    {
        _result = {};

        ExpandCluster expander(_player, _result, _board);
        for (u32 i = 0; i < _numPlayableSlots; ++i)
            expander.expandFrom(_playableSlots[i]);
    }

    //-------------------------------------------------------------------------------------------------
    float GameState::computeFreeSpaceHeuristic(Slot _player, float _weightCluster) const
    {
        u32 playerIndex = convertToIndex(_player);

        float numReachables = 0;
        for (u32 i = 0; i < m_reachableSlotsCache[playerIndex].m_numClusters; ++i)
        {
            u32 numPlayableInCluster = m_reachableSlotsCache[playerIndex].m_numPlayableSlotsPerCluster[i];
            float weight = _weightCluster > 0 ? powf(float(numPlayableInCluster) / (numPlayableInCluster + 1), _weightCluster) : 1;
            numReachables += m_reachableSlotsCache[playerIndex].m_clusterSize[i] * weight;
        }

        return numReachables;
    }

    //-------------------------------------------------------------------------------------------------
    static bool insertSorted(u32 _numSlots, Board::PlayableSlots& _array, ubyte2 _element)
    {
        auto begin = _array.begin();
        auto end = _array.begin() + _numSlots;

        auto insertionPoint = std::lower_bound(begin, end, _element);
        if (*insertionPoint == _element)
            return false;

        for (auto it = end; it != insertionPoint; --it)
            *it = *(it - 1);

        *insertionPoint = _element;
        return true;
    }

    //-------------------------------------------------------------------------------------------------
    void GameState::updatePlayablePositions(Slot _player, const Move& _move)
    {
        for (u32 p = 0; p < 4; ++p)
        {
            Board::PlayableSlots updatedPlayableSlots = {};
            u32 numSlots = 0;
            for (u32 i = 0; i < m_numPlayablePos[p]; ++i)
            {
                // The first turn for each player, we include the slot automatically
                if (getTurnCount() < p || m_board.isValidPlayableSlot(convertToSlot(p), m_playablePositions[p][i]))
                {
                    updatedPlayableSlots[numSlots++] = m_playablePositions[p][i];
                }
            }
            DEBUG_ASSERT(numSlots < 256);
            m_numPlayablePos[p] = ubyte(numSlots);
            memcpy(m_playablePositions[p].data(), updatedPlayableSlots.data(), numSlots * sizeof(ubyte2));
        }

        ivec2 iBoardPos = { int(_move.position.x), int(_move.position.y) };
        u32 playerIndex = convertToIndex(_player);
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
                            DEBUG_ASSERT(m_numPlayablePos[playerIndex] < Board::MaxPlayableCorners);
                            if (insertSorted(m_numPlayablePos[playerIndex], m_playablePositions[playerIndex], pos))
                                m_numPlayablePos[playerIndex]++;
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------------------------------------
    void BaseAI::start()
    {
        m_numHeuristicEvaluated = 0;
        m_numNodesExplored = 0;
        m_start = std::chrono::steady_clock::now();
    }

    void BaseAI::stop()
    {
        std::chrono::duration<float> diff = std::chrono::steady_clock::now() - m_start;
        m_timeInSecond = diff.count();
    }

    float BaseAI::nodePerSecond() const
    {
        return m_numNodesExplored / m_timeInSecond;
    }

    u32 BaseAI::getNumNodeExplored() const
    {
        return m_numNodesExplored;
    }
}
