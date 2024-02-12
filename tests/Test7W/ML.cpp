
#include "ML.h"

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

void ML_Toolbox::fillTensors(const Dataset& dataset, torch::Tensor& outData, torch::Tensor& outLabels)
{
	using namespace sevenWD;

	float* pData = new float[dataset.m_states.size() * GameState::TensorSize];
	float* pWritePtr = pData;
	float* pLabels = new float[dataset.m_winners.size()];

	for (size_t i = 0; i < dataset.m_winners.size(); ++i) {
		pLabels[i] = (dataset.m_winners[i] == 0) ? 1.0f : 0.0f;
		dataset.m_states[i].fillTensorData(pWritePtr, 0);
		pWritePtr += GameState::TensorSize;	
	}

	outData = torch::from_blob(pData, { int(dataset.m_winners.size()), sevenWD::GameState::TensorSize }, torch::kFloat).clone();
	outLabels = torch::from_blob(pLabels, { int(dataset.m_winners.size()), 1 }, torch::kFloat).clone();
	delete[] pData;
	delete[] pLabels;
}

void ML_Toolbox::Dataset::fillBatches(const sevenWD::GameContext& sevenWDContext, u32 batchSize, std::vector<Batch>& batches) const
{
	std::vector<u32> indexes(m_winners.size());
	for (u32 i = 0; i < indexes.size(); ++i)
		indexes[i] = i;

	std::shuffle(indexes.begin(), indexes.end(), sevenWDContext.rand());

	float* pData = new float[batchSize * sevenWD::GameState::TensorSize];
	float* pWritePtr = pData;
	float* pLabels = new float[batchSize];
	float* pWriteLabelsPtr = pLabels;

	for (u32 i = 0; i < indexes.size(); ++i) {
		*pWriteLabelsPtr = (m_winners[indexes[i]] == 0) ? 1.0f : 0.0f;
		m_states[indexes[i]].fillTensorData(pWritePtr, 0);

		pWritePtr += sevenWD::GameState::TensorSize;
		pWriteLabelsPtr++;

		if ((i + 1) % batchSize == 0) {
			batches.emplace_back();
			batches.back().data = torch::from_blob(pData, { int(batchSize), sevenWD::GameState::TensorSize }, torch::kFloat).clone();
			batches.back().labels = torch::from_blob(pLabels, { int(batchSize), 1 }, torch::kFloat).clone();
			pWritePtr = pData;
			pWriteLabelsPtr = pLabels;
		}
	}
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
void ML_Toolbox::trainNet(u32 epoch, const std::vector<Batch>& batches, T* pNet)
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
		
		std::cout << "Epoch:" << i << "/" << epoch << " | " << "Loss: " << avgLoss << " Precision : " << avgPrecision << std::endl;
	}
}

template void ML_Toolbox::trainNet<BaseLine>(u32 epoch, const std::vector<Batch>& batches, BaseLine* pNet);