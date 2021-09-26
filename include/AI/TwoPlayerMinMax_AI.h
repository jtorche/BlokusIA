#pragma once

#include "GenericMinMax_AI.h"

namespace blokusAI
{
    struct TwoPlayerMinMaxStrategy
    {
        static float computeScore(Slot _maxPlayer, BoardHeuristic _heuristic, const GameState& _gameState)
        {
            bool isP0P2_MaxPlayer = _maxPlayer == Slot::P0 || _maxPlayer == Slot::P2;
            return (isP0P2_MaxPlayer ? 1 : -1) * (_gameState.computeBoardScore(Slot::P0, _heuristic) +
                                                  _gameState.computeBoardScore(Slot::P2, _heuristic) -
                                                  _gameState.computeBoardScore(Slot::P1, _heuristic) -
                                                  _gameState.computeBoardScore(Slot::P3, _heuristic));
        }

        static bool isMaxPlayerTurn(Slot _maxPlayer, const GameState& _gameState)
        {
            return (u32(_maxPlayer) - u32(Slot::P0)) % 2 == _gameState.getPlayerTurn() % 2;
        }
    };

    using TwoPlayerMinMax_AI = GenericMinMax_AI<TwoPlayerMinMaxStrategy>;
}