
#include "ML.h"
#include <filesystem>

u32 ML_Toolbox::generateOneGameDatasSet(const sevenWD::GameContext& sevenWDContext, sevenWD::AIInterface* AIs[2], std::vector<sevenWD::GameState>(&data)[3], sevenWD::WinType& winType)
{
	using namespace sevenWD;
	GameController game(sevenWDContext);

	std::vector<Move> moves;
	Move move;
	do
	{
		// Sample state were player 0 has played, so its player 1 turn
		if (game.m_gameState.getCurrentPlayerTurn() == 1) {
			data[game.m_gameState.getCurrentAge()].push_back(game.m_gameState);
		}

		u32 curPlayerTurn = game.m_gameState.getCurrentPlayerTurn();
		game.enumerateMoves(moves);
		move = AIs[curPlayerTurn]->selectMove(sevenWDContext, game, moves);
	} while (!game.play(move));

	winType = game.m_winType;
	return game.m_state == GameController::State::WinPlayer0 ? 0 : 1;
}

#if 0
void ML_Toolbox::fillTensors(const Dataset& dataset, torch::Tensor& outData, torch::Tensor& outLabels)
{
	using namespace sevenWD;

	float* pData = new float[dataset.m_data.size() * GameState::TensorSize];
	float* pWritePtr = pData;
	float* pLabels = new float[dataset.m_data.size()];

	for (size_t i = 0; i < dataset.m_data.size(); ++i) {
		pLabels[i] = (dataset.m_data[i].m_winner == 0) ? 1.0f : 0.0f;
		dataset.m_data[i].m_state.fillTensorData(pWritePtr, 0);
		pWritePtr += GameState::TensorSize;	
	}

	outData = torch::from_blob(pData, { int(dataset.m_data.size()), sevenWD::GameState::TensorSize }, torch::kFloat).clone();
	outLabels = torch::from_blob(pLabels, { int(dataset.m_data.size()), 1 }, torch::kFloat).clone();
	delete[] pData;
	delete[] pLabels;
}
#endif

void ML_Toolbox::Dataset::fillBatches(u32 batchSize, std::vector<Batch>& batches, bool useExtraTensorData) const
{
	using namespace sevenWD;

	const u32 tensorSize = GameState::TensorSize + (useExtraTensorData ? GameState::ExtraTensorSize : 0);
	float* pData = new float[batchSize * tensorSize];
	float* pWritePtr = pData;
	float* pLabels = new float[batchSize];
	float* pWriteLabelsPtr = pLabels;

	for (u32 i = 0; i < m_data.size(); ++i) {
		*pWriteLabelsPtr = (m_data[i].m_winner == 0) ? 1.0f : 0.0f;
		m_data[i].m_state.fillTensorData(pWritePtr, 0);

		pWritePtr += sevenWD::GameState::TensorSize;
		if (useExtraTensorData)
		{
			m_data[i].m_state.fillExtraTensorData(pWritePtr);
			pWritePtr += GameState::ExtraTensorSize;
		}
		pWriteLabelsPtr++;

		if ((i + 1) % batchSize == 0) {
			batches.emplace_back();
			batches.back().data = torch::from_blob(pData, { int(batchSize), tensorSize }, torch::kFloat).clone();
			batches.back().labels = torch::from_blob(pLabels, { int(batchSize), 1 }, torch::kFloat).clone();
			pWritePtr = pData;
			pWriteLabelsPtr = pLabels;
		}
	}

	delete[] pData;
	delete[] pLabels;
}

float ML_Toolbox::evalPrecision(torch::Tensor predictions, torch::Tensor labels)
{
	predictions = torch::round(predictions);
	auto p = torch::mean(torch::abs(predictions - labels));

	return p.item<float>();
}

std::pair<float, float> ML_Toolbox::evalMeanLoss(torch::Tensor predictions, torch::Tensor labels, torch::Tensor weights)
{
	torch::NoGradGuard _{};
	torch::Tensor loss = torch::binary_cross_entropy(predictions, labels, weights);

	return { loss.item<float>(), evalPrecision(predictions, labels)};
}

template<typename T>
void ML_Toolbox::trainNet(u32 age, u32 epoch, const std::vector<Batch>& batches, T* pNet)
{
	using OptimizerType = torch::optim::AdamW;
	std::unique_ptr<OptimizerType> optimizer = std::make_unique<OptimizerType>(pNet->parameters(), 1e-4);

	for (u32 i = 0; i < epoch; ++i)
	{
		float avgLoss = 0;
		float avgPrecision = 0;

		for (u32 b = 0; b < batches.size(); ++b)
		{
			optimizer->zero_grad();
			torch::Tensor prediction = pNet->forward(batches[b].data);
			torch::Tensor loss = torch::binary_cross_entropy(prediction, batches[b].labels);

			loss.backward();
			optimizer->step();

			avgLoss += loss.item<float>();
			avgPrecision += ML_Toolbox::evalPrecision(prediction, batches[b].labels);

			//constexpr u32 reportingInterval = 100;
			//if (b % reportingInterval == 0)
			//{
			//	avgLoss /= reportingInterval;
			//	avgPrecision /= reportingInterval;
			//
			//	std::cout << "Batch:" << b << "/" << batches.size() << " | " << "Loss: " << avgLoss << " Precision : " << avgPrecision << std::endl;
			//	optimizer->zero_grad();
			//	avgLoss = 0;
			//	avgPrecision = 0;
			//}
		}

		avgLoss /= batches.size();
		avgPrecision /= batches.size();
		
		std::cout << std::setprecision(4) << "Epoch:" << i << "/" << epoch << " | ";

		for (u32 j = 0; j < age; ++j)
			std::cout << "                                ";

		std::cout << "Loss:" << avgLoss << " | Acc: " << avgPrecision << std::endl;
	}
}

template void ML_Toolbox::trainNet<BaseLine>(u32 age, u32 epoch, const std::vector<Batch>& batches, BaseLine* pNet);
template void ML_Toolbox::trainNet<TwoLayers>(u32 age, u32 epoch, const std::vector<Batch>& batches, TwoLayers* pNet);

template<typename T>
void ML_Toolbox::saveNet(u32 generation, std::shared_ptr<T>(&net)[3])
{
	for (u32 i = 0; i < 3; ++i) {
		std::stringstream str;
		str << "../7wDataset/dataset_" << T::getNetName() << (net[i]->m_extraTensorData ? "_extra" : "") << "_gen" << generation << "_age" << i << ".bin";
		torch::save(net[i], str.str());
	}
}

template void ML_Toolbox::saveNet<BaseLine>(u32 generation, std::shared_ptr<BaseLine>(&net)[3]);
template void ML_Toolbox::saveNet<TwoLayers>(u32 generation, std::shared_ptr<TwoLayers>(&net)[3]);

template<typename T>
bool ML_Toolbox::loadNet(u32 generation, std::shared_ptr<T>(&net)[3], bool useExtraTensorData)
{
	for (u32 i = 0; i < 3; ++i) {
		std::stringstream str;
		str << "../7wDataset/dataset_" << T::getNetName() << (useExtraTensorData ? "_extra" : "") << "_gen" << generation << "_age" << i << ".bin";
		if (std::filesystem::exists(str.str()))
		{
			net[i] = std::make_shared<T>(useExtraTensorData);
			torch::load(net[i], str.str());
		}
		else return false;
	}

	return true;
}

template bool ML_Toolbox::loadNet<BaseLine>(u32 generation, std::shared_ptr<BaseLine>(&net)[3], bool useExtraTensorData);
template bool ML_Toolbox::loadNet<TwoLayers>(u32 generation, std::shared_ptr<TwoLayers>(&net)[3], bool useExtraTensorData);

template<typename T>
std::pair<sevenWD::AIInterface*, u32> ML_Toolbox::loadAIFromFile(bool useExtraTensorData)
{
	sevenWD::AIInterface* pAI = nullptr;
	u32 i = 0;

	for (; i < 100; ++i)
	{
		std::shared_ptr<T> net[3];
		if (loadNet<T>(i, net, useExtraTensorData))
		{
			std::stringstream networkName;
			networkName << T::getNetName() << (useExtraTensorData ? "_extra" : "") << "_gen" << i;
			delete pAI;
			pAI = new NetworkAI<T>(networkName.str(), net);
		}
		else {
			break;
		}
	}

	return std::make_pair(pAI, i);
}

template std::pair<sevenWD::AIInterface*, u32> ML_Toolbox::loadAIFromFile<BaseLine>(bool useExtraTensorData);
template std::pair<sevenWD::AIInterface*, u32> ML_Toolbox::loadAIFromFile<TwoLayers>(bool useExtraTensorData);