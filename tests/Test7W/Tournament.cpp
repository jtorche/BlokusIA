
#include "Tournament.h"

Tournament::Tournament()
{
	m_numStateToSamplePerGame.civil = 4;
	m_numStateToSamplePerGame.military = 8;
	m_numStateToSamplePerGame.science = 11;

	//m_numStateToSamplePerGame.civil = 1;
	//m_numStateToSamplePerGame.military = 1;
	//m_numStateToSamplePerGame.science = 1;
}

void Tournament::generateDataset(u32 numRound, sevenWD::GameContext& context)
{
	using namespace sevenWD;

	for (u32 round = 0; round < numRound; ++round)
	{
		// Match every AIs against every others
		for (u32 i = 0; i < m_AIs.size(); ++i)
		{
			for (u32 j = 0; j < m_AIs.size(); ++j)
			{
				//if (i == j) continue;

				AIInterface* AIs[2] = { m_AIs[i], m_AIs[j] };

				WinType winType;
				std::vector<GameState> states[3];
				u32 winner = ML_Toolbox::generateOneGameDatasSet(context, AIs, states, winType);
				m_numWins[winner == 0 ? i : j].first++;
				m_winTypes[winner == 0 ? i : j].incr(winType);
				m_numWins[i].second++;
				m_numWins[j].second++;

				for (u32 age = 0; age < 3; ++age)
				{
					std::vector<u32> turns(states[age].size());
					for (size_t t = 0; t < turns.size(); ++t)
						turns[t] = (u32)t;

					std::shuffle(turns.begin(), turns.end(), context.rand());
					for (u32 t = 0; t < std::min(m_numStateToSamplePerGame.get(winType), (u32)states[age].size()); ++t)
					{
						switch (winType) {
						case WinType::Civil:
							m_civilWin[age].m_states.push_back(states[age][turns[t]]);
							m_civilWin[age].m_winners.push_back(winner);
							m_civilWin[age].m_winTypes.push_back(winType);
							break;
						case WinType::Military:
							m_militaryWin[age].m_states.push_back(states[age][turns[t]]);
							m_militaryWin[age].m_winners.push_back(winner);
							m_militaryWin[age].m_winTypes.push_back(winType);
							break;
						case WinType::Science:
							m_scienceWin[age].m_states.push_back(states[age][turns[t]]);
							m_scienceWin[age].m_winners.push_back(winner);
							m_scienceWin[age].m_winTypes.push_back(winType);
							break;
						}
					}
				}
			}
		}
	}
}

void Tournament::removeWorstAI()
{
	auto it = std::min_element(m_numWins.begin(), m_numWins.end());
	size_t index = std::distance(m_numWins.begin(), it);
	m_numWins.erase(m_numWins.begin() + index);
	m_winTypes.erase(m_winTypes.begin() + index);

	delete m_AIs[index];
	m_AIs.erase(m_AIs.begin() + index);
}

void Tournament::fillDataset(ML_Toolbox::Dataset(&dataset)[3]) const
{
	for (u32 i = 0; i < 3; ++i) 
	{
		dataset[i] += m_civilWin[i];
		dataset[i] += m_militaryWin[i];
		dataset[i] += m_scienceWin[i];
	}
}

void Tournament::resetTournament()
{
	for (u32 i = 0; i < 3; ++i)
	{
		m_civilWin[i].clear();
		m_militaryWin[i].clear();
		m_scienceWin[i].clear();
	}

	m_numWins.clear();
	m_winTypes.clear();
	for (u32 i = 0; i < m_AIs.size(); ++i) {
		m_numWins.push_back(std::make_pair(0u, 0u));
		m_winTypes.emplace_back();
	}
}

void Tournament::print() const
{
	std::cout << "Tournament result:" << std::endl;
	for (u32 i = 0; i < m_AIs.size(); ++i)
		std::cout << m_AIs[i]->getName() << " : " << m_numWins[i].first << " / " << m_numWins[i].second << "(" << m_winTypes[i].civil << "," << m_winTypes[i].military << "," << m_winTypes[i].science << ")" << std::endl;

	std::cout << "Civil : " << m_civilWin[0].m_winners.size() << " Military : " << m_militaryWin[0].m_winners.size() << " Science : " << m_scienceWin[0].m_winners.size() << std::endl;
}