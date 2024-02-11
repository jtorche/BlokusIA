#pragma once

#include "AI/7WDuel/GameController.h"
#include "AI/7WDuel/AI.h"
#include "ML.h"

class Tournament
{
public:
	Tournament();

	void addAI(sevenWD::AIInterface* pAI) { m_AIs.push_back(pAI); m_numWins.push_back(std::make_pair(0u, 0u)); m_winTypes.emplace_back(); }

	void generateDataset(u32 size, sevenWD::GameContext& context);
	void fillDataset(ML_Toolbox::Dataset (&dataset)[3]) const;


	void print() const;

private:
	struct WinTypeCounter {
		u32 civil = 0;
		u32 military = 0;
		u32 science = 0;

		void incr(sevenWD::WinType type) {
			switch (type) {
			case sevenWD::WinType::Civil:
				civil++; break;
			case sevenWD::WinType::Military:
				military++; break;
			case sevenWD::WinType::Science:
				science++; break;
			}
		}

		u32 get(sevenWD::WinType type) const {
			switch (type) {
			case sevenWD::WinType::Civil:
				return civil;
			case sevenWD::WinType::Military:
				return military;
			case sevenWD::WinType::Science:
				return science;
			}
			return 0;
		}
	};

	std::vector<sevenWD::AIInterface*> m_AIs;
	std::vector<std::pair<u32, u32>> m_numWins;
	std::vector<WinTypeCounter> m_winTypes;

	ML_Toolbox::Dataset m_civilWin[3];
	ML_Toolbox::Dataset m_militaryWin[3];
	ML_Toolbox::Dataset m_scienceWin[3];
	WinTypeCounter m_numStateToSamplePerGame;
};
