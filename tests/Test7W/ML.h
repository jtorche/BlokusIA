#pragma once

#include "AI/7WDuel/GameController.h"
#include "AI/7WDuel/AI.h"
#include <torch/torch.h>

struct ML_Toolbox
{
	static u32 generateOneGameDatasSet(const sevenWD::GameContext& sevenWDContext, sevenWD::AIInterface* AIs[2], std::vector<sevenWD::GameState>(&states)[3], sevenWD::WinType& winType);

	static void fillTensors(torch::Tensor& data, torch::Tensor& predictions, torch::Tensor& weights, const std::vector<sevenWD::GameState>& states, const std::vector<int>& winners);
	static float evalPrecision(torch::Tensor predictions, torch::Tensor labels);

	static std::pair<float, float> evalMeanLoss(torch::Tensor predictions, torch::Tensor labels, torch::Tensor weights);
};

struct TwoLayers : torch::nn::Module
{
	TwoLayers(u32 _inChannelCount)
	{
		using namespace torch;
		fully1 = register_module("fully1", nn::Linear(_inChannelCount, 64));
		fully2 = register_module("fully2", nn::Linear(64, 1));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x)
	{
		using namespace torch::indexing;

		//x = x.reshape({ x.sizes()[0], channelCount * Board::BoardSize * Board::BoardSize });
		x = torch::relu(fully1->forward(x));
		return torch::sigmoid(fully2->forward(x));
	}

	torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
};

struct ThreeLayers : torch::nn::Module
{
	ThreeLayers(u32 _inChannelCount)
	{
		using namespace torch;
		fully1 = register_module("fully1", nn::Linear(_inChannelCount, 64));
		fully2 = register_module("fully2", nn::Linear(64, 32));
		fully3 = register_module("fully3", nn::Linear(32, 1));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x)
	{
		using namespace torch::indexing;

		//x = x.reshape({ x.sizes()[0], channelCount * Board::BoardSize * Board::BoardSize });
		x = torch::relu(fully1->forward(x));
		x = torch::relu(fully2->forward(x));
		return torch::sigmoid(fully3->forward(x));
	}

	torch::nn::Linear fully1 = nullptr, fully2 = nullptr, fully3 = nullptr;
};

struct BaseLine : torch::nn::Module
{
	BaseLine(u32 _inChannelCount)
	{
		using namespace torch;
		fully1 = register_module("fully1", nn::Linear(_inChannelCount, 1));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x)
	{
		using namespace torch::indexing;
		return torch::sigmoid(fully1->forward(x));
	}

	torch::nn::Linear fully1 = nullptr;
};