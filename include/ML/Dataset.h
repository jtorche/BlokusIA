#pragma once

#include "Core/Common.h"
#include "AI/BlokusAI.h"

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

		void serialize(std::string _path) const;
		bool read(std::string _path);

	private:
		std::vector<Entry> m_data;
	};
}