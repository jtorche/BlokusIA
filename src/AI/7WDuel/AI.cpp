#include "AI/7WDuel/AI.h"

namespace sevenWD
{
	Move NoBurnAI::selectMove(const GameContext& _sevenWDContext, const GameController&, const std::vector<Move>& _moves) 
	{

		std::vector<Move> cpyMoves = _moves;
		std::sort(cpyMoves.begin(), cpyMoves.end(), [&](const Move& _a, const Move& _b)
			{
				bool isBurnA = _a.action == Move::Burn;
				bool isBurnB = _b.action == Move::Burn;
				return (isBurnA == isBurnB) ? false : !isBurnA;
			});

		size_t i = 0;
		for (; i < cpyMoves.size(); ++i) {
			if (cpyMoves[i].action == Move::Burn) {
				break;
			}
		}

		if (i == 0) {
			return _moves[_sevenWDContext.rand()() % _moves.size()];
		}
		else {
			return i == 1 ? cpyMoves[0] : cpyMoves[_sevenWDContext.rand()() % (i - 1)];
		}
	}
}