#pragma once

#include "Core/Common.h"
#include "AI/BlokusAI.h"
#include <torch/torch.h>

namespace blokusAI
{
    struct NetJojo : torch::nn::Module
    {
        NetJojo(u32 _inChannelCount, bool _outputScore) : outputScore{ _outputScore }
        {
            using namespace torch;
            conv1 = register_module("conv1", nn::Conv2d(nn::Conv2dOptions(_inChannelCount, 64, 5).padding(2).stride(2)));
            conv2 = register_module("conv2", nn::Conv2d(nn::Conv2dOptions(64, 128, 3).padding(1)));
            conv3 = register_module("conv3", nn::Conv2d(nn::Conv2dOptions(128, 256, 3).padding(1)));
            maxPool = register_module("maxPool", nn::MaxPool2d(nn::MaxPool2dOptions(2)));
            fully1 = register_module("fully1", nn::Linear(6400, 512));
            fully2 = register_module("fully2", nn::Linear(512, _outputScore ? 1 : 2));
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

            if (outputScore)
            {
                // the result is a score so we can use the linear outpput, however to help a bit we clamp the value as  it is always between 0-1
                x = x.clamp(0, 1);
            }
            else
            {
                x = torch::sigmoid(x);
            }

            return x;
        }

        bool outputScore;
        torch::nn::Conv2d conv1 = nullptr, conv2 = nullptr, conv3 = nullptr;
        torch::nn::MaxPool2d maxPool = nullptr;
        torch::nn::Linear fully1 = nullptr, fully2 = nullptr;
        torch::nn::BatchNorm2d batch_norm = nullptr;
    };
}