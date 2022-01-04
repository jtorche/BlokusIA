#pragma once

#include "Core/Common.h"
#include "AI/BlokusAI.h"
#include "ML/Dataset.h"

namespace blokusAI
{
	class GameGenerator
	{
	public:
		struct AI
		{
			std::string m_aiName;
			std::unique_ptr<BaseAI> m_ai;
			u32 m_numMatchPlayed = 0;
			float m_score = 0;
		};

		GameGenerator() = default;

		u32 numAIs() const { return (u32)m_allAIs.size(); }
		const Dataset& getDataset() { return m_dataset; }
		void resetDataset() { m_dataset.clear(); }

		void addAI(std::string _name, BaseAI* _ai)
		{
			m_allAIs.emplace_back(std::make_unique<AI>(AI{ _name, std::unique_ptr<BaseAI>(_ai) }));
		}

		void playGame(u32 _ia0, u32 _ia1, u32 _ia2, u32 _ia3);

		void printResult() const;

	private:
		std::vector<std::unique_ptr<AI>> m_allAIs;
		Dataset m_dataset;
		std::mutex m_mutex;
	};
}