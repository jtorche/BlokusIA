#pragma once

#include "AI/7WDuel/GameController.h"
#include "AI/7WDuel/AI.h"
#include "ML.h"

class Tournament
{
public:
	Tournament();

	void addAI(sevenWD::AIInterface* pAI) { m_AIs.push_back(pAI); m_numWins.push_back(std::make_pair(0u, 0u)); m_winTypes.emplace_back(); }
	void generateDataset(sevenWD::GameContext& context, u32 datasetSize);
	void generateDatasetFromAI(sevenWD::GameContext& context, sevenWD::AIInterface* pAI, u32 datasetSize);
	void removeWorstAI();

	void fillDataset(ML_Toolbox::Dataset (&dataset)[3]) const;
	void resetTournament(float percentageOfGamesToKeep);

	void print() const;

private:
	static constexpr u32 NumStatesToSamplePerGame = 4;

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

	ML_Toolbox::Dataset m_dataset[3];
};
