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

	void Dataset::serialize(std::string _path, u32 _offset, u32 _numElements) const
	{
		FILE* f = fopen(_path.c_str(), "wb+");
		if (f)
		{
			DEBUG_ASSERT(_numElements > 0 && _offset < m_data.size());
			fwrite(m_data.data() + _offset, sizeof(Entry), std::min<u32>(m_data.size() - _offset, _numElements), f);
			fclose(f);
		}
	}

	bool Dataset::read(std::string _path)
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
		}
		else return false;

		return true;
	}

	void Dataset::merge(Dataset&& _other)
	{
		for (const auto& dat : _other.m_data)
			m_data.push_back(dat);
		_other.clear();
	}

	void Dataset::shuffle()
	{
		std::shuffle(m_data.begin(), m_data.end(), s_rand);
	}

	torch::Tensor Dataset::computeWeight() const
	{
		u32 p0RankDistribution[4] = {};
		for (const auto& dat : m_data)
		{
			for (u32 i = 0; i < 4; ++i)
			{
				if(dat.ranking[i] == Slot::P0)
					p0RankDistribution[i]++;
			}
		}

		u32 sum = std::accumulate(std::begin(p0RankDistribution), std::end(p0RankDistribution), 0);
		vec2 w = { float(p0RankDistribution[0]) / sum, float(p0RankDistribution[0] + p0RankDistribution[1]) / sum };

		return torch::from_blob(&w, { 2 }).clone();
	}

	Dataset::Batches Dataset::constructTensors(u32 _epochSize, uvec2 _turnRange, u32 _turnOffset, bool _useReachableCluster) const
	{
		std::vector<std::pair<torch::Tensor, torch::Tensor>> tensorData;

		const u32 numSlice = 2 + (_useReachableCluster ? 2 : 0);
		const u32 sliceStride = Board::BoardSize * Board::BoardSize; // in float count
		
		float* blob = new float[_epochSize * sliceStride * numSlice];
		float* blobCurData = blob;
		std::vector<float> labels;
		
		u32 fillIndex = 0;
		for (u32 i = 0; i < m_data.size(); ++i)
		{
			if (m_data[i].turn % 4 == _turnOffset && m_data[i].turn >= _turnRange.x && m_data[i].turn <= _turnRange.y)
			{
				const u32 playerIndex = 0;
				fillInputTensorData(m_data[i].board, playerIndex, _useReachableCluster, blobCurData);
				blobCurData += sliceStride * numSlice;

				u32 rankingLookup[4];
				for (u32 rank = 0; rank < 4; ++rank)
					rankingLookup[u32(m_data[i].ranking[rank]) - u32(Slot::P0)] = rank;

				labels.push_back(rankingLookup[playerIndex] == 0 ? 1 : 0); // first class to predict if first
				labels.push_back(rankingLookup[playerIndex] <= 1 ? 1 : 0); // second class to predict if first or second
				fillIndex++;
			}

			if (fillIndex == _epochSize)
			{
				tensorData.push_back(
				{
					torch::from_blob(blob, { _epochSize, numSlice, Board::BoardSize, Board::BoardSize }, torch::TensorOptions(torch::ScalarType::Float)).clone(),
					torch::from_blob(labels.data(), { _epochSize, 2 }, torch::TensorOptions(torch::ScalarType::Float)).clone()
				});

				fillIndex = 0;
				blobCurData = blob;
				labels.clear();
			}
		}

		delete[] blob;
		return tensorData;
	}

	u32 Dataset::computeInputTensorDataSize(bool _useReachableCluster)
	{
		const u32 sliceStride = Board::BoardSize * Board::BoardSize; // in float count
		const u32 numSlice = _useReachableCluster ? 4 : 2;
		return numSlice * sliceStride;
	}

	torch::Tensor Dataset::fillInputTensorData(const Board& _board, u32 _playerIndex, bool _useReachableCluster, float* _outData)
	{
		DEBUG_ASSERT(_playerIndex == 0);
		Board board = _board.rotatedBoard(Rotation(u32(Rotation::Rot_0) + _playerIndex));

		const ReachableSlots* _reachableSlots[4] = {};
		if (_useReachableCluster)
		{
			ReachableSlots reachableSlots[4];
			for (Slot s : { Slot::P0, Slot::P1, Slot::P2, Slot::P3 })
			{
				u32 index = u32(s) - u32(Slot::P0);
				Board::PlayableSlots playable;
				u32 numPlayable = board.computeValidSlotsForPlayer(s, playable);
				GameState::computeReachableSlots(reachableSlots[index], s, board, playable, numPlayable);
				_reachableSlots[index] = &reachableSlots[index];
			}
		}

		return fillInputTensorData(_playerIndex, _useReachableCluster, _board, _reachableSlots, _outData);
	}

	torch::Tensor Dataset::fillInputTensorData(const GameState& _state, u32 _playerIndex, bool _useReachableCluster, float* _outData)
	{
		const ReachableSlots* _reachableSlots[4] = {};

		if (_useReachableCluster)
		{
			if (_playerIndex == 0)
			{
				_reachableSlots[0] = &_state.getPlayableSlot(Slot::P0);
				_reachableSlots[1] = &_state.getPlayableSlot(Slot::P1);
				_reachableSlots[2] = &_state.getPlayableSlot(Slot::P2);
				_reachableSlots[3] = &_state.getPlayableSlot(Slot::P3);
			}
			else
			{
				Board board = _state.getBoard().rotatedBoard(Rotation(u32(Rotation::Rot_0) + _playerIndex));
				return fillInputTensorData(board, _playerIndex, _useReachableCluster, _outData);
			}
		}

		return fillInputTensorData(_playerIndex, _useReachableCluster, _state.getBoard(), _useReachableCluster ? _reachableSlots : nullptr, _outData);
	}

	torch::Tensor Dataset::fillInputTensorData(u32 _playerIndex, bool _useReachableCluster, const Board& _board, const ReachableSlots* _reachableSlots[], float* _outData)
	{
		const u32 sliceStride = Board::BoardSize * Board::BoardSize; // in float count
		Board board = _board.rotatedBoard(Rotation(u32(Rotation::Rot_0) + _playerIndex));

		for (u32 y = 0; y < Board::BoardSize; ++y)
			for (u32 x = 0; x < Board::BoardSize; ++x)
			{
				Slot s = board.getSlot(x, y);
				_outData[y * Board::BoardSize + x] = (s == Slot::P0) ? 1 : 0;
				_outData[sliceStride + y * Board::BoardSize + x] = (s != Slot::P0 && s != Slot::Empty) ? -1 : 0;
			}

		if (_useReachableCluster)
		{
			for (u32 y = 0; y < Board::BoardSize; ++y)
				for (u32 x = 0; x < Board::BoardSize; ++x)
				{
					_outData[sliceStride * 2 + y * Board::BoardSize + x] = _reachableSlots[0]->m_clusters[x][y] > 0 ? 1 : 0;
					_outData[sliceStride * 3 + y * Board::BoardSize + x] = ((_reachableSlots[1]->m_clusters[x][y] > 0 ? 1 : 0) +
																		    (_reachableSlots[2]->m_clusters[x][y] > 0 ? 1 : 0) +
																		    (_reachableSlots[3]->m_clusters[x][y] > 0 ? 1 : 0)) / (-3.f);
				}
		}

		const u32 numSlice = 2 + (_useReachableCluster ? 2 : 0);
		return torch::from_blob(_outData, { 1, numSlice, Board::BoardSize, Board::BoardSize });
	}
}