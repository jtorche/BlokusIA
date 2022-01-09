#include "ML/cxxopts.h"

// Header
int genDataset(std::string _outputFolder, std::string _datasetBaseName, u32 _numDataset, u32 _numGamePerDataset);
int trainModel(std::string _datasetFolder, std::string _datasetBaseName, std::string _inModelPath, std::string _outModelPath, float _lr, uvec2 _turnRange, bool _useCluster);

int main(int argc, char * argv[]) 
{
    cxxopts::Options optionsBase("Blockus AI", "A program to generate blockus games and use it as a dataset to train a network.");
    optionsBase.add_options()
        ("g,genDataset", "Generate dataset")
        ("t,train", "Train a model");

    auto cmdArgParseRresult = optionsBase.parse(std::min(argc, 2), argv);
    if (cmdArgParseRresult["genDataset"].as<bool>())
    {
        cxxopts::Options optionsGen("Blockus AI", "Play random blockus games and generate a dataset.");
        optionsGen.allow_unrecognised_options();
        optionsGen.add_options()
            ("folder", "Output folder", cxxopts::value<std::string>())
            ("name", "Base name for dataset", cxxopts::value<std::string>()->default_value("dataset"))
            ("numDatasets", "Num dataset to generate", cxxopts::value<u32>()->default_value("64"))
            ("numGames", "Num games to play per dataset", cxxopts::value<u32>()->default_value("5000"));

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
            ("folder", "Output folder", cxxopts::value<std::string>())
            ("name", "Base name for dataset", cxxopts::value<std::string>()->default_value("dataset"))
            ("input", "Input model", cxxopts::value<std::string>()->default_value(""))
            ("output", "Output model", cxxopts::value<std::string>())
            ("lr", "Learning rate", cxxopts::value<float>()->default_value("0.02"))
            ("turnRange", "Game position to select in turn range", cxxopts::value<std::vector<u32>>()->default_value("0,80"))
            ("cluster", "Use cluster data to train");

        auto parseResult = optionsTrain.parse(argc, argv);

        std::vector<u32> turnRange = parseResult["turnRange"].as<std::vector<u32>>();
        if (turnRange.size() < 2)
            turnRange = { 0,80 };

        system("pause");
        return trainModel(parseResult["folder"].as<std::string>(),
                          parseResult["name"].as<std::string>(),
                          parseResult["input"].as<std::string>(),
                          parseResult["output"].as<std::string>(),
                          parseResult["lr"].as<float>(),
                          uvec2(turnRange[0], turnRange[1]),
                          parseResult["cluster"].as<bool>());

    }

    return 0;
}