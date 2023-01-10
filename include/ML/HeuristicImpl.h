#pragma once

#include "AI/blockusAI/BlokusAI.h"
#include "ML/NetworkDef.h"

namespace blokusAI
{
	using std::string;
	using std::vector;
	using std::pair;

	struct CustomHeuristicImpl final : CustomHeuristicInterface
	{
		CustomHeuristicImpl(string _path, vector<u32> _turnSplit, BlokusNet::Model _model, bool _useCluster = true);

		float moveHeuristic(const GameState&, const Move& _move, ubyte2 _playablePos) override;
		float boardHeuristic(const GameState&, Slot _player) override;

		vector<u32> m_netPerTurn;
		vector<std::array<std::shared_ptr<BlokusNet>, 4>> m_networks;
		bool m_useCluster;
	};
}