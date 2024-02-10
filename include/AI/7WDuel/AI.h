#pragma once

#include "AI/7WDuel/GameController.h"
#include <sstream>

namespace sevenWD
{
	struct AIInterface 
	{
		virtual Move selectMove(const GameContext& _sevenWDContext, const GameController& _game, const std::vector<Move>& _moves) = 0;
		virtual std::string getName() const = 0;
	};

	struct RandAI : AIInterface
	{
		Move selectMove(const GameContext& _sevenWDContext, const GameController&, const std::vector<Move>& _moves) override {
			return _moves[_sevenWDContext.rand()() % _moves.size()];
		}

		std::string getName() const {
			return "RandAI";
		}
	};

	struct NoBurnAI : AIInterface
	{
		Move selectMove(const GameContext& _sevenWDContext, const GameController&, const std::vector<Move>& _moves) override;

		std::string getName() const {
			return "NoBurnAI";
		}
	};

	struct MixAI : AIInterface {
		unsigned int m_precentage;
		std::unique_ptr<AIInterface> m_AIs[2];

		template<typename AI0, typename AI1>
		MixAI(AI0* pAI0, AI1* pAI1, unsigned int _precentage) : m_precentage(_precentage)
		{
			m_AIs[0] = std::unique_ptr<AI0>(pAI0);
			m_AIs[1] = std::unique_ptr<AI1>(pAI1);
		}

		Move selectMove(const GameContext& _sevenWDContext, const GameController& _game, const std::vector<Move>& _moves) override {
			return m_precentage < _sevenWDContext.rand()() % 100 ? m_AIs[0]->selectMove(_sevenWDContext, _game, _moves) : m_AIs[1]->selectMove(_sevenWDContext, _game, _moves);
		}

		std::string getName() const {
			std::ostringstream stringStream;
			stringStream << "MixAI(" << m_AIs[0]->getName() << "," << m_AIs[1]->getName() << ")";
			return stringStream.str();
		}
	};
}