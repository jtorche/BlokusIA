#include "ML/Dataset.h"

#include <fstream>

namespace blokusAI
{
	void Dataset::add(const GameState& _gameState, std::array<Slot, 4> _ranking)
	{
		Entry entry;
		entry.board = _gameState.getBoard();
		entry.turn = _gameState.getTurnCount();

		for (Slot p : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
			entry.playedTiles[u32(p) - u32(Slot::P0)] = (ubyte)_gameState.getPlayedPieceTiles(p);

		entry.ranking = _ranking;

		m_data.emplace_back(entry);
	}

	void Dataset::serialize(std::string _path) const
	{
		std::ios_base::openmode mode = std::ios_base::binary | std::ios_base::trunc;
		std::ofstream file(_path, mode);
		file.write((const char *)m_data.data(), m_data.size() * sizeof(Entry));
	}

	bool Dataset::read(std::string _path)
	{
		std::ios_base::openmode mode = std::ios_base::binary;
		std::ifstream file(_path);
		if (file)
		{
			std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(file), {});
			DEBUG_ASSERT(buffer.size() % sizeof(Entry) == 0);
			m_data.resize(buffer.size() / sizeof(Entry));
			memcpy(m_data.data(), buffer.data(), buffer.size());
			return true;
		}
		return false;
	}
}