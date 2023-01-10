#include "ML/HeuristicImpl.h"
#include "ML/Dataset.h"
#include <optional>
#include <filesystem>

namespace blokusAI
{
	CustomHeuristicImpl::CustomHeuristicImpl(string _path, vector<u32> _turnSplit, BlokusNet::Model _model, bool _useCluster) : m_useCluster{ _useCluster }
	{
		string modelName = "";
		switch (_model)
		{
		default:
			DEBUG_ASSERT(false);
		case BlokusNet::Model::Model_Baseline:
			modelName = "baseline"; break;
		case BlokusNet::Model::Model_Jojo:
			modelName = "jojo"; break;
		case BlokusNet::Model::Model_SimpleCnn:
			modelName = "simplecnn"; break;
		case BlokusNet::Model::Model_SimpleCnn2:
			modelName = "simplecnn2"; break;
		case BlokusNet::Model::Model_Cnn1:
			modelName = "cnn1"; break;
		case BlokusNet::Model::Model_TwoLayers:
			modelName = "twolayers"; break;
		}

		m_netPerTurn = std::move(_turnSplit);
		m_networks.reserve(m_netPerTurn.size());

		u32 prevTurnSplit = 0;
		for (u32 turn : m_netPerTurn)
		{
			m_networks.push_back({ std::make_shared<BlokusNet>(_model, _useCluster ? 4 : 2),
								   std::make_shared<BlokusNet>(_model, _useCluster ? 4 : 2),
								   std::make_shared<BlokusNet>(_model, _useCluster ? 4 : 2),
								   std::make_shared<BlokusNet>(_model, _useCluster ? 4 : 2) });

			for (u32 i = 0; i < 4; ++i)
			{
				string completePath = _path + "_" + std::to_string(prevTurnSplit) + "_" + std::to_string(turn) + "_" + modelName + "_" + std::to_string(i) + ".pt";
				torch::load(m_networks.back()[i], completePath);
			}

			prevTurnSplit = turn + 1;
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
		torch::NoGradGuard _{};

		u32 playerIndex = u32(_player) - u32(Slot::P0);
		u32 numFloats = Dataset::computeInputTensorDataSize(m_useCluster);
		auto data = std::unique_ptr<float[]>(new float[numFloats]);
		torch::Tensor tensor = Dataset::fillInputTensorData(_state, playerIndex, m_useCluster, data.get());

		u32 numTurnToWaitBeforePlay = ( (4 + playerIndex) - (_state.getTurnCount()%4) ) % 4;

		std::optional<torch::Tensor> result;
		for (u32 i = 0; i < m_netPerTurn.size(); ++i)
		{
			if (_state.getTurnCount() <= m_netPerTurn[i])
			{
				result = m_networks[i][numTurnToWaitBeforePlay]->forward(tensor);
				break;
			}
		}

		if (!result)
			result = m_networks.back()[numTurnToWaitBeforePlay]->forward(tensor);

		using namespace torch::indexing;
		float probaFirst = result->accessor<float, 2>()[0][0];
		//float probaFirstOrSecond = result->accessor<float, 2>()[0][1];

		return probaFirst;
	}
}