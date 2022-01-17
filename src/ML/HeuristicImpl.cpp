#include "ML/HeuristicImpl.h"
#include "ML/Dataset.h"
#include <optional>
#include <filesystem>

namespace blokusAI
{
	CustomHeuristicImpl::CustomHeuristicImpl(vector<pair<string, u32>> _netPathAndTurn, bool _useCluster) : m_useCluster{ _useCluster }
	{
		m_netPerTurn.reserve(_netPathAndTurn.size());
		m_networks.reserve(_netPathAndTurn.size());

		for (const auto& [path, turn] : _netPathAndTurn)
		{
			m_networks.emplace_back(std::make_shared<BlokusNet>(BlokusNet::Model::Model_Jojo, _useCluster ? 4 : 2));
			m_netPerTurn.push_back(turn);

			torch::load(m_networks.back(), path);
		}
	}

	float CustomHeuristicImpl::moveHeuristic(const GameState& _state, const Move& _move, ubyte2 _playablePos)
	{
		if (_state.getTurnCount() < 4)
			return _state.computeHeuristic(_move, _playablePos, MoveHeuristic::TileCount_DistCenter);
		return boardHeuristic(_state.play(_move), Slot(u32(Slot::P0) + _state.getPlayerTurn()));
	}

	float CustomHeuristicImpl::boardHeuristic(const GameState& _state, Slot _player)
	{
		u32 numFloats = Dataset::computeInputTensorDataSize(m_useCluster);
		auto data = std::unique_ptr<float[]>(new float[numFloats]);
		torch::Tensor tensor = Dataset::fillInputTensorData(_state.getBoard(), u32(_player) - u32(Slot::P0), m_useCluster, data.get());

		std::optional<torch::Tensor> result;
		for (u32 i = 0; i < m_netPerTurn.size(); ++i)
		{
			if (_state.getTurnCount() <= m_netPerTurn[i])
			{
				result = m_networks[i]->forward(tensor);
				break;
			}
		}

		if(!result)
			m_networks.back()->forward(tensor);

		using namespace torch::indexing;
		float probaFirst = result->accessor<float, 2>()[0][0];
		float probaFirstOrSecond = result->accessor<float, 2>()[0][1];

		return probaFirst;
	}
}