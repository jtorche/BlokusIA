
#include "ML.h"

u32 ML_Toolbox::generateOneGameDatasSet(const sevenWD::GameContext& sevenWDContext, sevenWD::AIInterface* AIs[2], std::vector<sevenWD::GameState> (&states)[3], sevenWD::WinType& winType)
{
	using namespace sevenWD;
	GameController game(sevenWDContext);

	std::vector<Move> moves;
	Move move;
	do
	{
		u32 curPlayerTurn = game.m_gameState.getCurrentPlayerTurn();
		game.enumerateMoves(moves);
		move = AIs[curPlayerTurn]->selectMove(sevenWDContext, game, moves);

		if (game.m_gameState.getCurrentPlayerTurn() == 0) {
			states[game.m_gameState.getCurrentAge()].push_back(game.m_gameState);
		}
	} 
	while (!game.play(move));

	winType = game.m_winType;
	return game.m_state == GameController::State::WinPlayer0 ? 0 : 1;
}

void ML_Toolbox::fillTensors(torch::Tensor& data, torch::Tensor& labels, torch::Tensor& outWeights, const std::vector<sevenWD::GameState>& states, const std::vector<int>& winners)
{
	float* pData = new float[states.size() * sevenWD::GameState::TensorSize];
	float* pLabels = new float[winners.size()];
	float weights[2] = {0,0};
	for (size_t i = 0; i < winners.size(); ++i) {
		pLabels[i] = winners[i] == 0 ? 1.0f : 0.0f;
		states[i].fillTensorData(pData + i * sevenWD::GameState::TensorSize);
		weights[winners[i]] += 1.0f;
	}

	weights[0] /= winners.size();
	weights[1] /= winners.size();
	outWeights = torch::from_blob(weights, 1, torch::kFloat).clone();

	data = torch::from_blob(pData, { int(winners.size()), sevenWD::GameState::TensorSize }, torch::kFloat).clone();
	labels = torch::from_blob(pLabels, { int(winners.size()), 1 }, torch::kFloat).clone();
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