#include "ML/Dataset.h"

#include <fstream>
#include <filesystem>

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

		u32 filesize = std::filesystem::file_size(_path);
		if (filesize > 0)
		{
			DEBUG_ASSERT(filesize % sizeof(Entry) == 0);
			std::ifstream file(_path);
			m_data.resize(filesize / sizeof(Entry));
			file.read((char*)m_data.data(), filesize);
			return true;
			
		}
		return false;
	}

	std::vector<torch::Tensor> Dataset::constructTensors(u32 _epochSize, uvec2 _turnRange, bool _useReachableCluster) const
	{
		std::vector<torch::Tensor> tensorData;

		const u32 numSlice = 2 + (_useReachableCluster ? 1 : 0);
		const u32 sliceStride = Board::BoardSize * Board::BoardSize; // in float count
		
		u32 globalIndex = 0;
		float* blob = new float[_epochSize * sliceStride * numSlice];
		float* blobCurData = blob;
		
		while(globalIndex < m_data.size() * 4)
		{
			u32 dataIndex = globalIndex / 4;
			u32 playerIndex = globalIndex % 4;

			if (m_data[dataIndex].turn >= _turnRange.x && m_data[dataIndex].turn <= _turnRange.y)
			{
				Board board = m_data[dataIndex].board.rotatedBoard(Rotation(u32(Rotation::Rot_0) + playerIndex));

				for (u32 y = 0; y < Board::BoardSize; ++y)
					for (u32 x = 0; x < Board::BoardSize; ++x)
					{
						Slot s = board.getSlot(x, y);
						blobCurData[y * Board::BoardSize + x] = (s == Slot::P0) ? 1 : 0;
						blobCurData[sliceStride + y * Board::BoardSize + x] = (s != Slot::P0 && s != Slot::Empty) ? 1 : 0;
						DEBUG_ASSERT(!_useReachableCluster);
					}

				blobCurData += sliceStride * numSlice;
			}

			globalIndex++;

			if (blob + u64(_epochSize * sliceStride * numSlice) == blobCurData)
			{
				tensorData.push_back(torch::from_blob(blob, { _epochSize, numSlice, Board::BoardSize, Board::BoardSize }, torch::TensorOptions(torch::ScalarType::Float)).clone());
				blobCurData = blob;
			}
			
		}

		delete[] blob;
		return tensorData;
	}
}