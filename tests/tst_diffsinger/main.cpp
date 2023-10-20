#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

static int routine(const ParseResult &result) {
    auto dsPath = result.valueForOption("--ds-file").toString();
    auto dsConfigPath = result.valueForOption("--acoustic-config").toString();
    auto vocoderConfigPath = result.valueForOption("--vocoder-config").toString();
    auto spkMixStr = result.valueForOption("--spk").toString();
    auto outputAudioTitle = result.valueForOption("--out").toString();
    auto speedup = result.valueForOption("--speedup").toInt();
    auto depth = result.valueForOption("--depth").toInt();
    auto cpuOnly = result.optionIsSet("--cpu-only");
    auto deviceIndex = result.valueForOption("--device-index").toInt();

    std::cout << "dsPath: " << dsPath << std::endl;
    std::cout << "dsConfigPath: " << dsConfigPath << std::endl;
    std::cout << "vocoderConfigPath: " << vocoderConfigPath << std::endl;
    std::cout << "spkMixStr: " << spkMixStr << std::endl;
    std::cout << "outputAudioTitle: " << outputAudioTitle << std::endl;
    std::cout << "speedup: " << speedup << std::endl;
    std::cout << "depth: " << depth << std::endl;
    std::cout << "cpuOnly: " << cpuOnly << std::endl;
    std::cout << "deviceIndex: " << deviceIndex << std::endl;

    return 0;
}

int main(int argc, char *argv[]) {
    SYSCMDLINE_UNUSED(argc);
    SYSCMDLINE_UNUSED(argv);

    Option dsFileOption("--ds-file", "Path to .ds file");
    dsFileOption.addArgument(Argument("file"));
    dsFileOption.setRequired(true);

    Option acousticConfigOption("--acoustic-config", "Path to acoustic dsconfig.yaml");
    acousticConfigOption.addArgument(Argument("file"));
    acousticConfigOption.setRequired(true);

    Option vocoderConfigOption("--vocoder-config", "Path to vocoder.yaml");
    vocoderConfigOption.addArgument(Argument("file"));
    vocoderConfigOption.setRequired(true);

    Option spkOption(
        "--spk", R"(Speaker Mixture (e.g. "name" or "name1|name2" or "name1:0.25|name2:0.75"))");
    spkOption.addArgument(Argument("spk"));

    Option outOption("--out", "Output Audio Filename (*.wav)");
    outOption.addArgument(Argument("file"));
    outOption.setRequired(true);

    Option speedUpOption("--speedup", "PNDM speedup ratio");
    speedUpOption.addArgument(Argument("rate", {}, 10));

    Option depthOption("--depth", "Shallow diffusion depth (needs acoustic model support)");
    depthOption.addArgument(Argument("depth", {}, 1000));
    depthOption.setShortMatchRule(Option::ShortMatchAll);

    Option cpuOnlyOption("--cpu-only", "Use CPU for audio inference");

    Option deviceIndexOption("--device-index", "GPU device index");
    deviceIndexOption.addArgument(Argument("index", {}, 1));

    Command rootCommand("DiffSinger");
    rootCommand.setOptions({
        dsFileOption,
        acousticConfigOption,
        vocoderConfigOption,
        spkOption,
        outOption,
        speedUpOption,
        depthOption,
        cpuOnlyOption,
        deviceIndexOption,
    });

    rootCommand.addVersionOption("0.0.0.1");
    rootCommand.addHelpOption(true);
    rootCommand.setHandler(routine);

    CommandCatalogue cc;
    cc.addOptionCategory("Required Options", {
                                                 "--ds-file",
                                                 "--acoustic-config",
                                                 "--vocoder-config",
                                                 "--out",
                                             });
    rootCommand.setCatalogue(cc);

    Parser parser(rootCommand);
    // parser.setShowHelpOnError(false);
    return parser.invoke(commandLineArguments());
}