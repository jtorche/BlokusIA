#pragma once

#include "Core/Common.h"
#include "AI/BlokusAI.h"
#include <torch/torch.h>

namespace blokusAI
{
	class Dataset
	{
	public:
		using Batches = std::vector<std::pair<torch::Tensor, torch::Tensor>>;

		struct Entry
		{
			u32 turn = 0;
			std::array <ubyte, 4> playedTiles = {};
			std::array<Slot, 4> ranking = { Slot::P0, Slot::P1, Slot::P2, Slot::P3 };
			Board board;
		};

		static_assert(sizeof(Entry) == sizeof(u32)*3 + sizeof(Board));

		void add(const GameState&, std::array<Slot, 4> _ranking);
		void clear() { m_data.clear(); }
		u32 count() { return u32(m_data.size()); }

		void serialize(std::string _path, u32 _offset = 0, u32 _numElements = std::numeric_limits<u32>::max()) const;
		bool read(std::string _path);
		void merge(Dataset&& _other);
		void shuffle();
		torch::Tensor computeWeight() const;

		Batches constructTensors(u32 _epochSize, uvec2 _turnRange = { 0, 84 }, u32 _turnOffset = 0, bool _useReachableCluster = false) const;

		static torch::Tensor fillInputTensorData(const Board& _board, u32 _playerIndex, bool _useReachableCluster, float* _data);
		static torch::Tensor fillInputTensorData(const GameState& _state, u32 _playerIndex, bool _useReachableCluster, float* _data);
		static u32 computeInputTensorDataSize(bool _useReachableCluster);

	private:
		std::vector<Entry> m_data;

		static torch::Tensor fillInputTensorData(u32 _playerIndex, bool _useReachableCluster, const Board& _board, const ReachableSlots * _reachableSlots[], float* _data);
	};
}