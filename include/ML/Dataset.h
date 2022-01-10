#pragma once

#include "Core/Common.h"
#include "AI/BlokusAI.h"
#include <torch/torch.h>

namespace blokusAI
{
	class Dataset
	{
	public:
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

		std::vector<std::pair<torch::Tensor, torch::Tensor>> constructTensors(u32 _epochSize, uvec2 _turnRange = { 0, 84 }, bool _useReachableCluster = false, bool _labelAsScore = false) const;

	private:
		std::vector<Entry> m_data;
	};
}