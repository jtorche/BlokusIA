#pragma once

#include "GenericMinMax_AI.h"

namespace blokusAI
{
    struct ParanoidStrategy
    {
        static float computeScore(Slot _maxPlayer, BoardHeuristic _heuristic, const GameState& _gameState, CustomHeuristicInterface* _custom)
        {
            return  (_maxPlayer == Slot::P0 ? 1 : -1) * _gameState.computeBoardScore(Slot::P0, _heuristic, _custom) +
                    (_maxPlayer == Slot::P1 ? 1 : -1) * _gameState.computeBoardScore(Slot::P1, _heuristic, _custom) +
                    (_maxPlayer == Slot::P2 ? 1 : -1) * _gameState.computeBoardScore(Slot::P2, _heuristic, _custom) +
                    (_maxPlayer == Slot::P3 ? 1 : -1) * _gameState.computeBoardScore(Slot::P3, _heuristic, _custom);
        }
  
        static bool isMaxPlayerTurn(Slot _maxPlayer, const GameState& _gameState)
        {
            return (u32(_maxPlayer) - u32(Slot::P0)) == _gameState.getPlayerTurn();
        }
    };

    struct FastParanoidStrategy
    {
        static float computeScore(Slot _maxPlayer, BoardHeuristic _heuristic, const GameState& _gameState, CustomHeuristicInterface * _custom)
        {
            return  _gameState.computeBoardScore(_maxPlayer, _heuristic, _custom);
        }

        static bool isMaxPlayerTurn(Slot _maxPlayer, const GameState& _gameState)
        {
            return (u32(_maxPlayer) - u32(Slot::P0)) == _gameState.getPlayerTurn();
        }
    };

    using ParanoidFourPlayer_AI = GenericMinMax_AI<ParanoidStrategy>;
    using FastParanoidFourPlayer_AI = GenericMinMax_AI<FastParanoidStrategy>;
}
