#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

int main(int /* argc */, char * /* argv */[]) {
    Argument srcArg("src", "Source files");
    srcArg.setDisplayName("<files>");
    srcArg.setMultiValueEnabled(true);

    Argument numArg("nums");
    numArg.setMultiValueEnabled(true);
    Option numOpt("-n", "Numbers");
    numOpt.addArguments({numArg});

    Argument destArg("dest", "Destination directory");

    Command rootCommand("work", "What to do on which day?");
    rootCommand.addArguments({srcArg, destArg});
    rootCommand.addOptions({numOpt});
    rootCommand.addHelpOption();
    rootCommand.setHandler([](const ParseResult &result) {
        u8printf("Sources:\n");
        for (const auto &item : result.valuesForArgument("src")) {
            u8printf("    %s\n", item.toString().data());
        }
        u8printf("Destination:\n");
        u8printf("    %s\n", result.valueForArgument("dest").toString().data());
        u8printf("Numbers:\n");
        for (const auto &item : result.resultForOption("-n").valuesForArgument("nums")) {
            u8printf("    %s\n", item.toString().data());
        }
        return 0;
    });

    Parser parser(rootCommand);
    return parser.invoke(SysCmdLine::commandLineArguments());
}