#pragma once

#include <bitset>
#include <chrono>
#include <atomic>
#include <random>

#include "BlokusGame.h"
#include "BlokusGameHelpers.h"
#include "Core/thread_pool.h"

namespace blokusAI
{
	extern PieceSymetries s_allPieces;
    extern u32 s_totalPieceTileCount;
    extern thread_pool s_threadPool;
    extern std::default_random_engine s_rand;
	void initBlokusAI();
    void printAllPieces();

    class GameStateCache;
    GameStateCache& getGlobalCache();

	//-------------------------------------------------------------------------------------------------
	struct Move
	{
		Piece piece;
		u32 pieceIndex = u32(-1);
		ubyte2 position;

		bool isValid() const { return pieceIndex != u32(-1); }
	};

    enum class BoardHeuristic
    {
        Custom,
        RemainingTiles,
        ReachableEmptySpace,
        ReachableEmptySpaceWeighted,
        ReachableEmptySpaceWeighted2,
        ReachableEmptySpaceOnly,
        ReachableEmptySpaceWeightedOnly,
        ReachableEmptySpaceWeighted2Only,
        BoardHeuristic_Count,
    };

    enum class MoveHeuristic
    {
        TileCount,
        TileCount_DistCenter,
        WeightedReachableSpace,
        ExtendingReachableSpace,
        Custom,
        MultiSource,
        MultiSource_Custom,
    };

    class GameState;
    struct Move;
    struct CustomHeuristicInterface
    {
        virtual float moveHeuristic(const GameState&, const Move& _move, ubyte2 _playablePos) = 0;
        virtual float boardHeuristic(const GameState&, Slot _player) = 0;
    };

	//-------------------------------------------------------------------------------------------------
    struct ExpandCluster;
    struct ReachableSlots
    {
        u32 m_numClusters = 0;
        ubyte m_clusters[Board::BoardSize][Board::BoardSize] = { {0} };
        ubyte m_numPlayableSlotsPerCluster[(Board::BoardSize * Board::BoardSize) / 4] = {};
        u16 m_clusterSize[(Board::BoardSize * Board::BoardSize) / 4] = {};
    };

    struct MultiSourceMoveHeuristicParam
    {
        u32 m_numPiecesWithBridge = 16;
        u32 m_numPieceAtCenter = 16;
    };

	class GameState
	{
	public:
        static const float s_endGameScore;

		GameState();
		GameState play(const Move&) const;
        GameState skip() const;

        bool operator==(const GameState&) const;

		const Board& getBoard() const { return m_board; }
		u32 getPlayerTurn() const { return m_turn % 4; }
		u32 getTurnCount() const { return m_turn; }
        bool noMoveLeft(Slot _player) const { return m_remainingPieces[u32(_player) - u32(Slot::P0)].test(BlokusGame::PiecesCount); }

        template<typename Functor>
        void visitMoves(u32 _playerTurn, Functor&&) const;

		std::vector<std::pair<Move, float>> enumerateMoves(MoveHeuristic _moveHeuristic, CustomHeuristicInterface * _customHeuristic = nullptr) const;
        void findCandidatMoves(u32 _numMoves, std::vector<std::pair<Move, float>>& _allMoves, u32 _numTurnToForceBestMoveHeuristic) const;
        std::vector<std::pair<Move, float>> findMovesToLookAt(MoveHeuristic _moveHeuristic, u32 _numMoves, u32 _numTurnToForceBestHeuristic,
                                                              const MultiSourceMoveHeuristicParam * _multiSrcParam = nullptr, CustomHeuristicInterface* _customHeuristic = nullptr) const;

        u32 getPlayedPieceTiles(Slot _player) const;
        const ReachableSlots& getPlayableSlot(Slot _player) const;

		float computeHeuristic(const Move& _move, ubyte2 _playablePos, MoveHeuristic) const;
		float computeBoardScore(Slot _player, BoardHeuristic, CustomHeuristicInterface* _customHeuristic = nullptr) const;

        static u32 getBestMoveIndex(const std::vector<float>&, u32 _amongNBestMoves);

        static void computeReachableSlots(ReachableSlots& _result, Slot _player, const Board& _board, const Board::PlayableSlots& _playableSlots, u32 _numPlayableSlots);

	private:
		Board m_board;
		std::bitset<BlokusGame::PiecesCount + 1> m_remainingPieces[4]; // last bit to store if a player can't play anymore
		u32 m_turn = 0;
        u32 m_playedTiles[4] = {};
        // Accumulate a value to compensate the space lost by playing big pieces
        // in order to still favor big pieces in "space based" heuristic
        u32 m_pieceSpaceScoreCompensation[4] = {}; 

        // Cache playable positions per players
        ubyte m_numPlayablePos[4] = {};
        Board::PlayableSlots m_playablePositions[4];

        // Cache of reachable slots per players
        ReachableSlots m_reachableSlotsCache[4];

        float computeBoardScoreInner(Slot _player, BoardHeuristic, CustomHeuristicInterface* _customHeuristic) const;
        void computeReachableSlots(Slot _player, ExpandCluster& _expander) const;
        float computeFreeSpaceHeuristic(Slot _player, float _weightCluster) const;

        void updatePlayablePositions(Slot _player, const Move& _move);

        friend struct std::hash<blokusAI::GameState>;
        friend class GameStateCache;
	};

    template<typename Functor>
    void GameState::visitMoves(u32 _playerTurn, Functor&& _functor) const
    {
        if (m_remainingPieces[_playerTurn].test(BlokusGame::PiecesCount))
            return;

        Slot playerToMove = convertToSlot(_playerTurn);

        const Board::PlayableSlots& slots = m_playablePositions[_playerTurn];
        u32 numSlots = m_numPlayablePos[_playerTurn];

        for (u32 i = 0; i < numSlots; ++i)
        {
            for (auto it = s_allPieces.rbegin(); it != s_allPieces.rend(); ++it)
            {
                u32 piece = (u32)std::distance(s_allPieces.begin(), it.base()) - 1;
                if (m_remainingPieces[_playerTurn].test(piece))
                {
                    for (const Piece& p : *it)
                    {
                        std::array<ubyte2, Piece::MaxPlayableCorners> pieceMoves;
                        u32 numMoveForPiece = m_board.getPiecePlayablePositions(playerToMove, p, slots[i], pieceMoves, m_turn < 4);

                        for (u32 j = 0; j < numMoveForPiece; ++j)
                        {
                            Move move = { p, piece, pieceMoves[j] };
                            if (!_functor(move, slots[i]))
                                return;
                        }
                    }
                }
            }
        }
    }

    //-------------------------------------------------------------------------------------------------
    struct BaseAI
    {
        struct Parameters
        {
            CustomHeuristicInterface* customHeuristic = nullptr;
            u32 maxMoveToLookAt = 16;
            u32 maxMoveInRecursion = 16;
            BoardHeuristic heuristic = BoardHeuristic::ReachableEmptySpaceWeighted;
            MoveHeuristic moveHeuristic = MoveHeuristic::TileCount;
            MultiSourceMoveHeuristicParam multiSourceParam = {};
            u32 maxDepth = 1;
            u32 selectAmongNBestMoves = 1;
            u32 numTurnToForceBestMoveHeuristic = 3;
            bool monothread = true;
        };

        BaseAI(const Parameters& _parameters) : m_params{ _parameters } {}
        virtual ~BaseAI() = default;

        Parameters m_params;

        std::atomic<bool> m_stopAI = false;

        std::atomic<u32> m_numNodesExplored = 0;
        std::atomic<u32> m_numHeuristicEvaluated = 0;

        std::chrono::steady_clock::time_point m_start;
        float m_timeInSecond = 0;

        void start();
        void stop();

        float nodePerSecond() const;
        u32 getNumNodeExplored() const;

        virtual std::pair<Move, float> findBestMove(const GameState& _gameState) = 0;
    };
}

//-------------------------------------------------------------------------------------------------
namespace std
{
    template<> struct hash<blokusAI::GameState>
    {
        size_t operator()(const blokusAI::GameState& _key) const
        {
            size_t h = 0;
            core::hash_combine(h, _key.getTurnCount());
            core::hash_combine(h, _key.getBoard());
            for(const auto& bitSet : _key.m_remainingPieces)
                core::hash_combine(h, bitSet.to_ulong());

            return h;
        }
    };
}
