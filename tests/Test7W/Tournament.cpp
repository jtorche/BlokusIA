
#include "Tournament.h"
#include <execution>

Tournament::Tournament()
{

}

void Tournament::generateDataset(sevenWD::GameContext& context, u32 datasetSize)
{
	using namespace sevenWD;

	while(m_dataset[0].m_data.size() < datasetSize)
	{
		// Match every AIs against every others
		for (u32 i = 0; i < m_AIs.size(); ++i)
		{
			for (u32 j = 0; j < m_AIs.size(); ++j)
			{
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
					for (u32 t = 0; t < std::min(NumStatesToSamplePerGame, (u32)states[age].size()); ++t)
					{
						ML_Toolbox::Dataset::Point p{ states[age][turns[t]], winner, winType };
						m_dataset[age].m_data.push_back(p);
					}
				}
			}
		}
	}

	m_dataset[0].shuffle(context);
	m_dataset[1].shuffle(context);
	m_dataset[2].shuffle(context);
}

void Tournament::generateDatasetFromAI(sevenWD::GameContext& context, sevenWD::AIInterface* pAI, u32 datasetSize)
{
	using namespace sevenWD;

	addAI(pAI);

	std::atomic_uint numGameInDataset = (u32)m_dataset[0].m_data.size();
	std::vector<std::array<ML_Toolbox::Dataset, 3>> perThreadDataset(16); // 16 threads

	std::for_each(std::execution::par, perThreadDataset.begin(), perThreadDataset.end(), [&](auto& threadSafeDataset)
	{
		u32 parity = 0;
		while (numGameInDataset < datasetSize)
		{
			// Match the last AI against every others
			for (u32 i = 0; i < m_AIs.size() - 1; ++i)
			{
				AIInterface* AIs[2] = { pAI, m_AIs[i] };
				u32 aiIndex[2] = { u32(m_AIs.size() - 1), i };
				if (parity)
				{
					std::swap(AIs[0], AIs[1]);
					std::swap(aiIndex[0], aiIndex[1]);
				}

				WinType winType;
				std::vector<GameState> states[3];
				u32 winner = ML_Toolbox::generateOneGameDatasSet(context, AIs, states, winType);

				m_numWins[aiIndex[winner]].first++;
				m_winTypes[aiIndex[winner]].incr(winType);
				m_numWins[aiIndex[0]].second++;
				m_numWins[aiIndex[1]].second++;

				for (u32 age = 0; age < 3; ++age)
				{
					std::vector<u32> turns(states[age].size());
					for (size_t t = 0; t < turns.size(); ++t)
						turns[t] = (u32)t;

					std::shuffle(turns.begin(), turns.end(), context.rand());
					for (u32 t = 0; t < std::min(NumStatesToSamplePerGame, (u32)states[age].size()); ++t)
					{
						ML_Toolbox::Dataset::Point p{ states[age][turns[t]], winner, winType };
						threadSafeDataset[age].m_data.push_back(p);
						numGameInDataset++;
					}
				}
			}

			parity = (parity + 1) % 2;
		}
	});

	for (const auto& dataset : perThreadDataset)
	{
		m_dataset[0] += dataset[0];
		m_dataset[1] += dataset[1];
		m_dataset[2] += dataset[2];
	}

	m_dataset[0].shuffle(context);
	m_dataset[1].shuffle(context);
	m_dataset[2].shuffle(context);
}

void Tournament::removeWorstAI(u32 amountOfAIsToKeep)
{
	while (m_AIs.size() > amountOfAIsToKeep) {
		auto it = std::min_element(m_numWins.begin(), m_numWins.end(), [](const auto& a, const auto& b)
			{
				return (double(a.first) / a.second) < (double(b.first) / b.second);
			});

		size_t index = std::distance(m_numWins.begin(), it);
		m_numWins.erase(m_numWins.begin() + index);
		m_winTypes.erase(m_winTypes.begin() + index);

		delete m_AIs[index];
		m_AIs.erase(m_AIs.begin() + index);
	}
}

void Tournament::fillDataset(ML_Toolbox::Dataset(&dataset)[3]) const
{
	for (u32 i = 0; i < 3; ++i) 
		dataset[i] += m_dataset[i];
}

void Tournament::resetTournament(float percentageOfGamesToKeep)
{
	for (u32 i = 0; i < 3; ++i) {
		m_dataset[i].m_data.resize(size_t(((double)m_dataset[i].m_data.size()) * percentageOfGamesToKeep));
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
	{
		std::cout << m_AIs[i]->getName() << " : Winrate " << std::setprecision(2) << float(m_numWins[i].first) / m_numWins[i].second << " ; "
			      << m_numWins[i].first << " / " << m_numWins[i].second << "(" << m_winTypes[i].civil << "," << m_winTypes[i].military << "," << m_winTypes[i].science << ")" << std::endl;
	}
}