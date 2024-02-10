
#include "AI/7WDuel/GameController.h"
#include "AI/7WDuel/AI.h"

u32 generateOneGameDatasSet(const sevenWD::GameContext& sevenWDContext, sevenWD::AIInterface* AIs[2], std::vector<sevenWD::GameState> (&states)[3], sevenWD::WinType& winType)
{
	using namespace sevenWD;
	GameController game(sevenWDContext);

	std::vector<Move> moves;
	Move move;
	do
	{
		u32 curPlayerTurn = game.m_gameState.getCurrentPlayerTurn();
		game.enumerateMoves(moves);
		move = AIs[curPlayerTurn]->selectMove(sevenWDContext, game, moves);

		if (game.m_gameState.getCurrentPlayerTurn() == 0) {
			states[game.m_gameState.getCurrentAge()].push_back(game.m_gameState);
		}
	} 
	while (!game.play(move));

	winType = game.m_winType;
	return game.m_state == GameController::State::WinPlayer0 ? 0 : 1;
}