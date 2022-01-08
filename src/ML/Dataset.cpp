#include "ML/Dataset.h"

#include <filesystem>
#include <random>

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
		FILE* f = fopen(_path.c_str(), "wb+");
		if (f)
		{
			fwrite(m_data.data(), sizeof(Entry), m_data.size(), f);
			fclose(f);
		}
	}

	bool Dataset::read(std::string _path, bool _shuffle)
	{
		std::ios_base::openmode mode = std::ios_base::binary;

		u32 filesize = std::filesystem::file_size(_path);
		if (filesize > 0)
		{
			FILE* f = fopen(_path.c_str(), "rb");
			if (f)
			{
				DEBUG_ASSERT(filesize % sizeof(Entry) == 0);
				m_data.resize(filesize / sizeof(Entry));
				fread(m_data.data(), sizeof(Entry), m_data.size(), f);
				fclose(f);
			}
			else return false;

			for (auto& d : m_data)
				DEBUG_ASSERT(d.turn != 0);
		}
		else return false;

		if (_shuffle)
		{
			std::shuffle(m_data.begin(), m_data.end(), s_rand);
		}
		return true;
	}

	std::vector<std::pair<torch::Tensor, torch::Tensor>> Dataset::constructTensors(u32 _epochSize, uvec2 _turnRange, bool _useReachableCluster, bool _labelAsScore) const
	{
		std::vector<std::pair<torch::Tensor, torch::Tensor>> tensorData;

		const u32 numSlice = 2 + (_useReachableCluster ? 1 : 0);
		const u32 sliceStride = Board::BoardSize * Board::BoardSize; // in float count
		
		u32 globalIndex = 0;
		float* blob = new float[_epochSize * sliceStride * numSlice];
		float* blobCurData = blob;
		std::vector<float> labels;
		
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
				
				u32 rankingLookup[4];
				for (u32 i = 0; i < 4; ++i)
					rankingLookup[u32(m_data[dataIndex].ranking[i]) - u32(Slot::P0)] = i;

				if (_labelAsScore)
				{
					auto getScore = [&]() -> float
					{
						switch (rankingLookup[playerIndex])
						{
						case 0: return 1;
						case 1: return 0.5;
						case 2:
						case 3: return 0;
						}
					};
					labels.push_back(getScore());
				}
				else
				{
					labels.push_back(rankingLookup[playerIndex] == 0 ? 1 : 0); // first class to predict if first
					labels.push_back(rankingLookup[playerIndex] <= 1 ? 1 : 0); // second class to predict if first or second
				}
			}

			globalIndex++;

			if (blob + u64(_epochSize * sliceStride * numSlice) == blobCurData)
			{
				tensorData.push_back(
				{
					torch::from_blob(blob, { _epochSize, numSlice, Board::BoardSize, Board::BoardSize }, torch::TensorOptions(torch::ScalarType::Float)).clone(),
					_labelAsScore ? torch::from_blob(labels.data(), { _epochSize }, torch::TensorOptions(torch::ScalarType::Float)).clone() :
									torch::from_blob(labels.data(), { _epochSize, 2 }, torch::TensorOptions(torch::ScalarType::Float)).clone()
				});

				blobCurData = blob;
				labels.clear();
			}
			
		}

		delete[] blob;
		return tensorData;
	}
}