#pragma once

#include <bitset>
#include <chrono>
#include <atomic>

#include "BlokusGame.h"
#include "BlokusGameHelpers.h"
#include "Core/thread_pool.h"

namespace blokusAI
{
	extern PieceSymetries s_allPieces;
    extern u32 s_totalPieceTileCount;
    extern thread_pool s_threadPool;
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
        WeightedReachableSpace,
        ExtendingReachableSpace,
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

	class GameState
	{
	public:
        static const u32 s_NumTurnToRushCenter = 3;
        static const float s_endGameScore;

		GameState();
		GameState play(const Move&) const;
        GameState skip() const;

        bool operator==(const GameState&) const;

		const Board& getBoard() const { return m_board; }
		u32 getPlayerTurn() const { return m_turn % 4; }
		u32 getTurnCount() const { return m_turn; }
        bool noMoveLeft(Slot _player) const { return m_remainingPieces[u32(_player) - u32(Slot::P0)].test(BlokusGame::PiecesCount); }

		std::vector<std::pair<Move, float>> enumerateMoves(MoveHeuristic _moveHeuristic) const;
        void findCandidatMoves(u32 _numMoves, std::vector<std::pair<Move, float>>& _allMoves) const;
        u32 getPlayedPieceTiles(Slot _player) const;

		float computeHeuristic(const Move& _move, ubyte2 _playablePos, MoveHeuristic) const;
		float computeBoardScore(Slot _player, BoardHeuristic) const;

        static u32 getBestMoveIndex(const std::vector<float>&);

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

        float computeBoardScoreInner(Slot _player, BoardHeuristic) const;
        void computeReachableSlots(Slot _player, ExpandCluster& _expander) const;
        float computeFreeSpaceHeuristic(Slot _player, float _weightCluster) const;

        void updatePlayablePositions(Slot _player, const Move& _move);

        friend struct std::hash<blokusAI::GameState>;
        friend class GameStateCache;
	};

    //-------------------------------------------------------------------------------------------------
    struct BaseAI
    {
        std::atomic<bool> m_stopAI = false;

        std::atomic<u32> m_numNodesExplored = 0;
        std::atomic<u32> m_numHeuristicEvaluated = 0;

        std::chrono::steady_clock::time_point m_start;
        float m_timeInSecond = 0;

        void start();
        void stop();

        float nodePerSecond() const;
        u32 getNumNodeExplored() const;
        u32 maxMoveToLookAt(const GameState& _state) const;
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
