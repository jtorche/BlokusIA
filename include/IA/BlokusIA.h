#pragma once

#include <bitset>
#include <chrono>
#include <atomic>

#include "BlokusGame.h"
#include "BlokusGameHelpers.h"
#include "Core/thread_pool.h"

namespace BlokusIA
{
	extern PieceSymetries s_allPieces;
    extern u32 s_totalPieceTileCount;
    extern thread_pool s_threadPool;
	void initBlokusIA();

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
        NumberOfMoves,
        ReachableEmptySpace,
        ReachableEmptySpaceWeighted,
        ReachableEmptySpaceWeighted2,
    };

	//-------------------------------------------------------------------------------------------------
    struct ExpandCluster;

	class GameState
	{
	public:
		GameState();
		GameState play(const Move&) const;
        GameState skip() const;

		const Board& getBoard() const { return m_board; }
		u32 getPlayerTurn() const { return m_turn % 4; }
		u32 getTurnCount() const { return m_turn; }

		std::vector<Move> enumerateMoves(bool _sortByHeuristic) const;

		float computeHeuristic(const Move& _move) const;
		float computeBoardScore(Slot _player, BoardHeuristic) const;
		float computeScoreUpperBound(Slot _player, BoardHeuristic) const;
        float computeScoreLowerBound(Slot _player, BoardHeuristic) const;

	private:
		Board m_board;
		std::bitset<BlokusGame::PiecesCount> m_remainingPieces[4];
		u32 m_turn = 0;

        void computeReachableSlots(Slot _player, ExpandCluster& _expander) const;
        float computeFreeSpaceHeuristic(Slot _player, float _weightCluster) const;
        u32 getPlayedPieceTiles(Slot _player) const;
	};

    //-------------------------------------------------------------------------------------------------
    struct BaseIA
    {
        std::atomic<bool> m_stopIA = false;

        std::atomic<u32> m_numNodesExplored = 0;
        std::atomic<u32> m_numHeuristicEvaluated = 0;

        std::chrono::steady_clock::time_point m_start;
        float m_timeInSecond = 0;

        void start();
        void stop();

        float nodePerSecond() const;
        size_t maxMoveToLookAt(const GameState& _state) const;
    };
}