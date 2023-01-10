#pragma once

#include "Core/Common.h"
#include "AI/blockusAI/BlokusAI.h"
#include <torch/torch.h>

namespace blokusAI
{
    struct Baseline
    {
        Baseline(torch::nn::Module * _torchModule, u32 _inChannelCount) : channelCount{ _inChannelCount }
        {
            using namespace torch;
            fully = _torchModule->register_module("fully", nn::Linear(_inChannelCount * Board::BoardSize * Board::BoardSize, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            using namespace torch::indexing;
            
            x = x.reshape({ x.sizes()[0], channelCount * Board::BoardSize * Board::BoardSize });
            return  torch::sigmoid(fully->forward(x));

            return x;
        }

        u32 channelCount;
        torch::nn::Linear fully = nullptr;
    };

    struct TwoLayers
    {
        TwoLayers(torch::nn::Module* _torchModule, u32 _inChannelCount) : channelCount{ _inChannelCount }
        {
            using namespace torch;
            fully1 = _torchModule->register_module("fully1", nn::Linear(_inChannelCount * Board::BoardSize * Board::BoardSize, 64));
            fully2 = _torchModule->register_module("fully2", nn::Linear(64, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            using namespace torch::indexing;

            x = x.reshape({ x.sizes()[0], channelCount * Board::BoardSize * Board::BoardSize });
            x  = torch::relu(fully1->forward(x));
            return torch::sigmoid(fully2->forward(x));
        }

        u32 channelCount;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    };

    struct SimpleCnn
    {
        SimpleCnn(torch::nn::Module* _torchModule, u32 _inChannelCount)
        {
            using namespace torch;
            conv1 = _torchModule->register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 3).padding(1).stride(2)));
            pool = _torchModule->register_module("pool", nn::MaxPool2d(nn::MaxPool2dImpl({ 2,2 })));
            fully1 = _torchModule->register_module("fully1", nn::Linear(25 * 64, 64));
            fully2 = _torchModule->register_module("fully2", nn::Linear(64, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            using namespace torch::indexing;

            x = torch::relu(conv1->forward(x));
            x = pool->forward(x);
            x = x.reshape({ x.sizes()[0], 25 * 64 });
            x = torch::relu(fully1->forward(x));
            return torch::sigmoid(fully2->forward(x));
        }

        torch::nn::Conv2d conv1 = nullptr;
        torch::nn::MaxPool2d pool = nullptr;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    };

    struct SimpleCnn2
    {
        SimpleCnn2(torch::nn::Module* _torchModule, u32 _inChannelCount)
        {
            using namespace torch;
            conv1 = _torchModule->register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 45, 3).padding(1).stride(2)));
            conv2 = _torchModule->register_module("conv2", nn::Conv2d(nn::Conv2dOptions(45, 90, 3).padding(1).stride(2)));
            fully1 = _torchModule->register_module("fully1", nn::Linear(25 * 90, 45));
            fully2 = _torchModule->register_module("fully2", nn::Linear(45, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            using namespace torch::indexing;

            x = torch::relu(conv1->forward(x));
            x = torch::relu(conv2->forward(x));
            x = x.reshape({ x.sizes()[0], 25 * 90 });
            x = torch::relu(fully1->forward(x));
            return torch::sigmoid(fully2->forward(x));
        }

        torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    };

    struct Cnn1
    {
        Cnn1(torch::nn::Module* _torchModule, u32 _inChannelCount)
        {
            using namespace torch;
            conv1 = _torchModule->register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 3).padding(1).stride(2)));
            conv2 = _torchModule->register_module("conv2", nn::Conv2d(nn::Conv2dOptions(64, 128, 3).padding(1).stride(2)));
            fully1 = _torchModule->register_module("fully1", nn::Linear(25 * 128, 80));
            fully2 = _torchModule->register_module("fully2", nn::Linear(80, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            using namespace torch::indexing;

            x = torch::relu(conv1->forward(x));
            x = torch::relu(conv2->forward(x));
            x = x.reshape({ x.sizes()[0], 25 * 128 });
            x = torch::relu(fully1->forward(x));
            return torch::sigmoid(fully2->forward(x));
        }

        torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
    };

    struct NetJojo
    {
        NetJojo(torch::nn::Module * _torchModule, u32 _inChannelCount)
        {
            using namespace torch;
            conv1 = _torchModule->register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 5).padding(2).stride(2)));
            conv2 = _torchModule->register_module("conv2", nn::Conv2d(nn::Conv2dOptions(64, 128, 3).padding(1)));
            conv3 = _torchModule->register_module("conv3", nn::Conv2d(nn::Conv2dOptions(128, 256, 3).padding(1)));
            maxPool = _torchModule->register_module("maxPool", nn::MaxPool2d(nn::MaxPool2dOptions(2)));
            fully1 = _torchModule->register_module("fully1", nn::Linear(6400, 512));
            fully2 = _torchModule->register_module("fully2", nn::Linear(512, 2));
        }

        // Implement the Net's algorithm.
        torch::Tensor forward(torch::Tensor x)
        {
            x = torch::relu(conv1->forward(x));
            x = torch::relu(conv2->forward(x));
            x = torch::relu(conv3->forward(x));
            x = maxPool->forward(x);
            x = x.flatten(1);
            x = torch::relu(fully1->forward(x));
            x = fully2->forward(x);
            x = torch::sigmoid(x);

            return x;
        }

        torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr, conv3 = nullptr;
        torch::nn::MaxPool2d maxPool = nullptr;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
        torch::nn::BatchNorm2d batch_norm = nullptr;
    };

    struct BlokusNet : torch::nn::Module
    {
        enum class Model { Model_Baseline, Model_TwoLayers, Model_Jojo, Model_SimpleCnn, Model_SimpleCnn2, Model_Cnn1  };
        BlokusNet(Model _model, u32 _inChannelCount) : m_model{ _model }
        {
            switch (_model)
            {
            case Model::Model_Baseline:
                m_baseModel = std::make_unique<Baseline>(this, _inChannelCount); break;
            case Model::Model_TwoLayers:
                m_twoLayersModel = std::make_unique<TwoLayers>(this, _inChannelCount); break;
            case Model::Model_Jojo:
                m_jojoModel = std::make_unique<NetJojo>(this, _inChannelCount); break;
            case Model::Model_SimpleCnn:
                m_simpleCnnModel = std::make_unique<SimpleCnn>(this, _inChannelCount); break;
            case Model::Model_SimpleCnn2:
                m_simpleCnn2Model = std::make_unique<SimpleCnn2>(this, _inChannelCount); break;
            case Model::Model_Cnn1:
                m_cnn1Model = std::make_unique<Cnn1>(this, _inChannelCount); break;
            }
        }

        torch::Tensor forward(torch::Tensor x)
        {
            switch (m_model)
            {
            default:
            case Model::Model_Baseline:
                return m_baseModel->forward(x);
            case Model::Model_TwoLayers:
                return m_twoLayersModel->forward(x);
            case Model::Model_Jojo:
                return m_jojoModel->forward(x);
            case Model::Model_SimpleCnn:
                return m_simpleCnnModel->forward(x);
            case Model::Model_SimpleCnn2:
                return m_simpleCnn2Model->forward(x);
            case Model::Model_Cnn1:
                return m_cnn1Model->forward(x);
            }
        }

        Model m_model;
        std::unique_ptr<Baseline> m_baseModel;
        std::unique_ptr<TwoLayers> m_twoLayersModel;
        std::unique_ptr<NetJojo> m_jojoModel;
        std::unique_ptr<SimpleCnn> m_simpleCnnModel;
        std::unique_ptr<SimpleCnn2> m_simpleCnn2Model;
        std::unique_ptr<Cnn1> m_cnn1Model;
    };
}