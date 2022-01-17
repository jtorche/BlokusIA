#pragma once

#include "AI/BlokusAI.h"
#include "ML/NetworkDef.h"

namespace blokusAI
{
	using std::string;
	using std::vector;
	using std::pair;

	struct CustomHeuristicImpl final : CustomHeuristicInterface
	{
		CustomHeuristicImpl(vector<pair<string, u32>> _netPathAndTurn, BlokusNet::Model _model, bool _useCluster);

		float moveHeuristic(const GameState&, const Move& _move, ubyte2 _playablePos) override;
		float boardHeuristic(const GameState&, Slot _player) override;

		vector<u32> m_netPerTurn;
		vector<std::shared_ptr<BlokusNet>> m_networks;
		bool m_useCluster;
	};
}