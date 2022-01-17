#include "ML/GameGenerator.h"

namespace blokusAI
{
	void GameGenerator::playGame(u32 _ia0, u32 _ia1, u32 _ia2, u32 _ia3)
	{
		std::vector<GameState> gameStates;
		gameStates.reserve(22);
		GameState gameState;

		bool gameContinue = true;
		u32 turn = 0;
		u32 ias[4] = { _ia0, _ia1, _ia2, _ia3 };
		while (gameContinue)
		{
			if (turn > 0 && turn % 4 == 0)
				gameStates.push_back(gameState);

			u32 iaIndex = turn % 4;
			const auto& ai = m_allAIs[ias[iaIndex]]->m_ai;
			auto move_score = ai->findBestMove(gameState);
			
			Move move = move_score.first;
			if (move.isValid())
				gameState = gameState.play(move);
			else
				gameState = gameState.skip();

			u32 numPlayerStuck = 0;
			for(u32 i=0 ; i<4 ; ++i)
			{ 
				if (gameState.noMoveLeft(Slot(i + u32(Slot::P0))))
					numPlayerStuck++;
			}

			gameContinue = numPlayerStuck < 4;
			turn++;
		}

		std::array<Slot, 4> iaRanking = { Slot::P0, Slot::P1, Slot::P2, Slot::P3 };
		std::sort(std::begin(iaRanking), std::end(iaRanking), [&](Slot a, Slot b) { return gameState.getPlayedPieceTiles(a) > gameState.getPlayedPieceTiles(b); });
		
		float scorePerRank[4] = { 1, 0.5, 0.25, 0 };

		{
			std::lock_guard _(m_mutex);
			for (u32 i = 0; i < 4; ++i)
			{
				u32 iaIndex = u32(iaRanking[i]) - u32(Slot::P0);
				m_allAIs[ias[iaIndex]]->m_numMatchPlayed++;
				m_allAIs[ias[iaIndex]]->m_score += scorePerRank[i];
			}

			// fill dataset
			for (const auto& savedState : gameStates)
			{
				m_dataset.add(savedState, iaRanking);
			}
		}
	}

	void GameGenerator::printResult() const
	{
		for (const auto& ai : m_allAIs)
		{
			std::cout << ai->m_aiName << " : " << ai->m_score << " / " << ai->m_numMatchPlayed << " = " << float(ai->m_score) / ai->m_numMatchPlayed << std::endl;
		}
	}
}