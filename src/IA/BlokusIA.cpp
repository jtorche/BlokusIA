#include "BlokusIA.h"
#include "BlokusGameHelpers.h"

namespace BlokusIA
{
	PieceSymetries s_allPieces = {};

	//-------------------------------------------------------------------------------------------------
	void initBlokusIA()
	{
		s_allPieces = Helpers::getAllPieceSymetries();
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
		TIM_ASSERT(m_remainingPieces[turn].test(_move.pieceIndex));
		newGameState.m_remainingPieces[turn].reset(_move.pieceIndex);

		Slot player = Slot(u32(Slot::P0) + turn);
		TIM_ASSERT(m_board.canAddPiece(player, _move.piece, { _move.position.x, _move.position.y }));
		newGameState.m_board.addPiece(player, _move.piece, _move.position);
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
			std::sort(std::begin(moves), std::end(moves), [this](const Move& m1, const Move& m2)
			{
				return computeHeuristic(m1) > computeHeuristic(m2);
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

		TIM_ASSERT(closestDistToCenter <= 1);

		return _move.piece.getNumTiles() * 10.0f +
			   _move.piece.getNumCorners() +
			   1.f - closestDistToCenter;
	}

	//-------------------------------------------------------------------------------------------------
	float GameState::computeBoardScore(Slot _player) const
	{
		float score = 0;

		u32 playerIndex = u32(_player) - u32(Slot::P0);
		for (u32 i = 0; i < m_remainingPieces[playerIndex].size(); ++i)
			if (m_remainingPieces[playerIndex].test(i))
				score += s_allPieces[i].begin()->getNumTiles();

		return score;
	}

	//-------------------------------------------------------------------------------------------------
	size_t TwoPlayerMinMaxIA::maxMoveToLookAt(const GameState& _gameState) const
	{
		if (_gameState.getTurnCount() < 16)
			return 8;
		else
			return 16;
	}

	//-------------------------------------------------------------------------------------------------
	Move TwoPlayerMinMaxIA::findBestMove(const GameState& _gameState)
	{
		auto moves = _gameState.enumerateMoves(true);
		moves.resize(std::min(moves.size(), maxMoveToLookAt(_gameState)));

		if (moves.empty())
			return {};

		std::vector<float> scores(moves.size());
		std::transform(moves.begin(), moves.end(), scores.begin(),
			[&](const Move& move) -> float
			{
				return evalPositionRec(false, _gameState.play(move), 0, { -std::numeric_limits<float>::max(), std::numeric_limits<float>::max() });
			});
		
		auto best = std::max_element(scores.begin(), scores.end());
		return moves[std::distance(scores.begin(), best)];
	}

	//-------------------------------------------------------------------------------------------------
	float TwoPlayerMinMaxIA::evalPositionRec(bool _isMaxPlayerTurn, const GameState& _gameState, u32 _depth, vec2 _a_b)
	{
		bool isP0P2_MaxPlayer = _isMaxPlayerTurn == (_gameState.getPlayerTurn() % 2 == 0);

		if (_depth >= m_maxDepth)
			return computeScore(isP0P2_MaxPlayer, _gameState);

		auto moves = _gameState.enumerateMoves(true);

		// The "maxPlayer" supposes the opponent minimize his score, reverse for the "minPlayer"
		float score = _isMaxPlayerTurn ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

		auto minmax = [_isMaxPlayerTurn](float s1, float s2)
		{
			return _isMaxPlayerTurn ? std::min(s1, s2) : std::max(s1, s2);
		};

		for (size_t i = 0; i < std::min(moves.size(), maxMoveToLookAt(_gameState)); ++i)
		{
			score = minmax(evalPositionRec(!_isMaxPlayerTurn, _gameState.play(moves[i]), _depth + 1, _a_b), score);
			if (_isMaxPlayerTurn)
			{
				if (score <= _a_b.x)
					return score;
				_a_b.y = std::min(_a_b.y, score);
			}
			else
			{
				if (score >= _a_b.y)
					return score;
				_a_b.x = std::max(_a_b.x, score);
			}
		}

		return score;
	}
}