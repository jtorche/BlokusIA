#pragma once

#include "AI/7WDuel/GameController.h"
#include "AI/7WDuel/AI.h"
#include <torch/torch.h>

struct ML_Toolbox
{ 
	struct Batch {
		torch::Tensor data;
		torch::Tensor labels;
	};
	struct Dataset
	{
		struct Point {
			sevenWD::GameState m_state;
			u32 m_winner;
			sevenWD::WinType m_winType;
		};
		std::vector<Point> m_data;

		void clear() {
			m_data.clear();
		}
		void shuffle(const sevenWD::GameContext& sevenWDContext) {
			std::shuffle(m_data.begin(), m_data.end(), sevenWDContext.rand());
		}
		void operator+=(const Dataset& dataset) {
			for (const Point& d : dataset.m_data)
				m_data.push_back(d);
		}

		void fillBatches(u32 batchSize, std::vector<Batch>& batches, bool useExtraTensorData) const;
	};

	static u32 generateOneGameDatasSet(const sevenWD::GameContext& sevenWDContext, sevenWD::AIInterface* AIs[2], std::vector<sevenWD::GameState>(&data)[3], sevenWD::WinType& winType);

	static void fillTensors(const Dataset& dataset, torch::Tensor& outData, torch::Tensor& outLabels);
	static float evalPrecision(torch::Tensor predictions, torch::Tensor labels);
	static std::pair<float, float> evalMeanLoss(torch::Tensor predictions, torch::Tensor labels, torch::Tensor weights);

	template<typename T>
	static void trainNet(u32 age, u32 epoch, const std::vector<Batch>& batches, T* pNet);

	template<typename T>
	static void saveNet(u32 generation, std::shared_ptr<T>(&net)[3]);
	template<typename T>
	static bool loadNet(u32 generation, std::shared_ptr<T>(&net)[3], bool useExtraTensorData);

	template<typename T>
	static std::pair<sevenWD::AIInterface*, u32> loadAIFromFile(bool useExtraTensorData);
	
};

struct TwoLayers : torch::nn::Module
{
	static const char* getNetName() { return "TwoLayers"; }

	TwoLayers(bool useExtraTensorData) : m_extraTensorData(useExtraTensorData)
	{
		using namespace torch;
		u32 tensorSize = sevenWD::GameState::TensorSize + (useExtraTensorData ? sevenWD::GameState::ExtraTensorSize : 0);
		fully1 = register_module("fully1", nn::Linear(tensorSize, 64));
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
	bool m_extraTensorData;
};

struct TwoLayers16 : torch::nn::Module
{
	static const char* getNetName() { return "TwoLayers16"; }

	TwoLayers16(bool useExtraTensorData) : m_extraTensorData(useExtraTensorData)
	{
		using namespace torch;
		u32 tensorSize = sevenWD::GameState::TensorSize + (useExtraTensorData ? sevenWD::GameState::ExtraTensorSize : 0);
		fully1 = register_module("fully1", nn::Linear(tensorSize, 16));
		fully2 = register_module("fully2", nn::Linear(16, 1));
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
	bool m_extraTensorData;
};

struct ThreeLayers : torch::nn::Module
{
	static const char* getNetName() { return "ThreeLayers"; }

	ThreeLayers(bool useExtraTensorData) : m_extraTensorData(useExtraTensorData)
	{
		using namespace torch;
		u32 tensorSize = sevenWD::GameState::TensorSize + (useExtraTensorData ? sevenWD::GameState::ExtraTensorSize : 0);
		fully1 = register_module("fully1", nn::Linear(tensorSize, 64));
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
	bool m_extraTensorData;
};

struct BaseLine : torch::nn::Module
{
	static const char* getNetName() { return "BaseLine"; }

	BaseLine(bool useExtraTensorData) : m_extraTensorData(useExtraTensorData)
	{
		using namespace torch;
		u32 tensorSize = sevenWD::GameState::TensorSize + (useExtraTensorData ? sevenWD::GameState::ExtraTensorSize : 0);
		fully1 = register_module("fully1", nn::Linear(tensorSize, 1));
	}

	// Implement the Net's algorithm.
	torch::Tensor forward(torch::Tensor x)
	{
		using namespace torch::indexing;
		return torch::sigmoid(fully1->forward(x));
	}

	torch::nn::Linear fully1 = nullptr;
	bool m_extraTensorData;
};

template<typename T>
struct NetworkAI : sevenWD::AIInterface
{
	NetworkAI(std::string name, std::shared_ptr<T>(&network)[3]) : m_name(name), m_network{ network[0], network[1], network[2] } {}

	sevenWD::Move selectMove(const sevenWD::GameContext& /*_sevenWDContext*/, const sevenWD::GameController& controller, const std::vector<sevenWD::Move>& _moves) override
	{
		std::vector<float> scores(_moves.size());

		for (u32 i = 0; i < _moves.size(); ++i) {
			sevenWD::GameController tmpController = controller;
			bool endGame = tmpController.play(_moves[i]);
			if (endGame) {
				u32 winner = (tmpController.m_state == sevenWD::GameController::State::WinPlayer0) ? 0 : 1;
				scores[i] = (controller.m_gameState.getCurrentPlayerTurn() == winner) ? 1.0f : 0.0f;
			} else {
				auto& network = m_network[tmpController.m_gameState.getCurrentAge()];

				const u32 tensorSize = sevenWD::GameState::TensorSize + (network->m_extraTensorData ? sevenWD::GameState::ExtraTensorSize : 0);
				std::unique_ptr<float[]> buffer = std::make_unique<float[]>(tensorSize);
				tmpController.m_gameState.fillTensorData(buffer.get(), controller.m_gameState.getCurrentPlayerTurn());
				if (network->m_extraTensorData)
					tmpController.m_gameState.fillExtraTensorData(buffer.get() + sevenWD::GameState::TensorSize);

				torch::Tensor result = m_network[tmpController.m_gameState.getCurrentAge()]->forward(torch::from_blob(buffer.get(), {1, tensorSize}, torch::kFloat));
				scores[i] = result[0].item<float>();
			}
		}
		
		auto it = std::max_element(scores.begin(), scores.end());
		return _moves[std::distance(scores.begin(), it)];
	}

	std::string getName() const override {
		return m_name;
	}

	std::string m_name;
	std::shared_ptr<T> m_network[3];
};

