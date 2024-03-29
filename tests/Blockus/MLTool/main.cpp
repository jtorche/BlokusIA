#include "ML/cxxopts.h"

// Header
int genDataset(std::string _outputFolder, std::string _datasetBaseName, u32 _numDataset, u32 _numGamePerDataset);
int trainModel(std::string _model, std::string _datasetFolder, std::string _datasetBaseName, std::string _testsetName, std::string _inModelPath, std::string _outModelPath, 
               u32 _datasetIndex, float _lr, uvec2 _turnRange, u32 _turnOffset, bool _useCluster, bool _autoLr, bool _autoExit);
int shuffleDataset(std::string _datasetFolder, std::string _datasetBaseName, u32 _numDatasetOut);

int main(int argc, char * argv[]) 
{
    cxxopts::Options optionsBase("Blockus AI", "A program to generate blockus games and use it as a dataset to train a network.");
    optionsBase.add_options()
        ("g,genDataset", "Generate dataset")
        ("t,train", "Train a model")
        ("h,trainHelper", "Train all models for a given net")
        ("s,shuffle", "Shuffle multiple dataset");

    auto cmdArgParseRresult = optionsBase.parse(std::min(argc, 2), argv);
    if (cmdArgParseRresult["genDataset"].as<bool>())
    {
        cxxopts::Options optionsGen("Blockus AI", "Play random blockus games and generate a dataset.");
        optionsGen.allow_unrecognised_options();
        optionsGen.add_options()
            ("folder", "Output folder", cxxopts::value<std::string>())
            ("name", "Base name for dataset", cxxopts::value<std::string>()->default_value("dataset"))
            ("numDatasets", "Num dataset to generate", cxxopts::value<u32>()->default_value("16"))
            ("numGames", "Num games to play per dataset", cxxopts::value<u32>()->default_value("10000"));

        auto parseResult = optionsGen.parse(argc, argv);

        system("pause");
        return genDataset(parseResult["folder"].as<std::string>(), 
                          parseResult["name"].as<std::string>(),
                          parseResult["numDatasets"].as<u32>(),
                          parseResult["numGames"].as<u32>());
    }
    else if (cmdArgParseRresult["train"].as<bool>())
    {
        cxxopts::Options optionsTrain("Blockus AI", "Play random blockus games and generate a dataset.");
        optionsTrain.allow_unrecognised_options();
        optionsTrain.add_options()
            ("model", "Model to use (baseline, jojo, fully)", cxxopts::value<std::string>()->default_value("baseline"))
            ("folder", "Output folder", cxxopts::value<std::string>())
            ("name", "Base name for dataset", cxxopts::value<std::string>()->default_value("dataset"))
            ("testset", "filename for a testset", cxxopts::value<std::string>()->default_value("testset"))
            ("input", "Input model", cxxopts::value<std::string>()->default_value(""))
            ("output", "Output model", cxxopts::value<std::string>())
            ("offset", "Offset to skip some dataset before training", cxxopts::value<u32>()->default_value("0"))
            ("lr", "Learning rate", cxxopts::value<float>()->default_value("0.001"))
            ("turnRange", "Game position to select in turn range", cxxopts::value<std::vector<u32>>()->default_value("0,80"))
            ("turn", "In how much turn the IA will play", cxxopts::value<u32>()->default_value("0"))
            ("cluster", "Use cluster data to train")
            ("autoLr", "Adapt Lr automatically, don't stop training")
            ("autoExit", "Exit when test set loss doesnt improve anymore");

        auto parseResult = optionsTrain.parse(argc, argv);

        std::vector<u32> turnRange = parseResult["turnRange"].as<std::vector<u32>>();
        if (turnRange.size() < 2)
            turnRange = { 0,80 };

        return trainModel(parseResult["model"].as<std::string>(),
                          parseResult["folder"].as<std::string>(),
                          parseResult["name"].as<std::string>(),
                          parseResult["testset"].as<std::string>(),
                          parseResult["input"].as<std::string>(),
                          parseResult["output"].as<std::string>(),
                          parseResult["offset"].as<u32>(),
                          parseResult["lr"].as<float>(),
                          uvec2(turnRange[0], turnRange[1]),
                          parseResult["turn"].as<u32>(),
                          parseResult["cluster"].as<bool>(), 
                          parseResult["autoLr"].as<bool>(),
                          parseResult["autoExit"].as<bool>());

    }
    else if (cmdArgParseRresult["shuffle"].as<bool>())
    {
        cxxopts::Options optionsTrain("Blockus AI", "Shuffle datasets.");
        optionsTrain.allow_unrecognised_options();
        optionsTrain.add_options()
            ("folder", "Output folder", cxxopts::value<std::string>())
            ("name", "Base name for dataset", cxxopts::value<std::string>()->default_value("dataset"))
            ("numDatasets", "Num dataset to generate", cxxopts::value<u32>()->default_value("8"));

        auto parseResult = optionsTrain.parse(argc, argv);

        system("pause");
        return shuffleDataset(parseResult["folder"].as<std::string>(), parseResult["name"].as<std::string>(), parseResult["numDatasets"].as<u32>());
    }
    else if (cmdArgParseRresult["trainHelper"].as<bool>())
    {
        cxxopts::Options optionsTrain("Blockus AI", "Full train");
        optionsTrain.allow_unrecognised_options();
        optionsTrain.add_options()
            ("model", "Model to train", cxxopts::value<std::string>())
            ("dataFolder", "Output folder", cxxopts::value<std::string>())
            ("outputFolder", "Output folder", cxxopts::value<std::string>())
            ("lr", "Learning rate", cxxopts::value<float>()->default_value("0.0001"))
            ("split", "Split model per turn", cxxopts::value<std::vector<u32>>()->default_value("16,40,84"));

        auto parseResult = optionsTrain.parse(argc, argv);
        system("pause");

        std::string modelName = parseResult["model"].as<std::string>();
        std::string dataFolder = parseResult["dataFolder"].as<std::string>();
        std::string output = parseResult["outputFolder"].as<std::string>();
        float lr = parseResult["lr"].as<float>();
        std::vector<u32> split = parseResult["split"].as<std::vector<u32>>();

        auto runCmdLine = [&](u32 _turnRange0, u32 _turnRange1, u32 _turn)
        {
            std::string cmdLine = std::string(argv[0]) + " --train --folder=" + dataFolder +
                                  " --lr=" + std::to_string(lr) +
                                  " --intput=dummy" +
                                  " --output=" + output + "\\model_" + std::to_string(_turnRange0) + "_" + std::to_string(_turnRange1) +
                                  " --turnRange=" + std::to_string(_turnRange0) + "," + std::to_string(_turnRange1) +
                                  " --autoExit --cluster" +
                                  " --model=" + modelName + 
                                  " --turn=" + std::to_string(_turn);
            std::cout << cmdLine << std::endl;
            system(cmdLine.c_str());
        };

        runCmdLine(split[1] + 1, split[2], 0);
        runCmdLine(split[1] + 1, split[2], 1);
        runCmdLine(split[1] + 1, split[2], 2);
        runCmdLine(split[1] + 1, split[2], 3);

        runCmdLine(split[0] + 1, split[1], 0);
        runCmdLine(split[0] + 1, split[1], 1);
        runCmdLine(split[0] + 1, split[1], 2);
        runCmdLine(split[0] + 1, split[1], 3);

        runCmdLine(0, split[0], 0);
        runCmdLine(0, split[0], 1);
        runCmdLine(0, split[0], 2);
        runCmdLine(0, split[0], 3);
        return 0;
    }

    return 0;
}