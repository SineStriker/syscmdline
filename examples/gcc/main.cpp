#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

int main(int /* argc */, char * /* argv */[]) {
    Option linkDirOption("-L", "Add link directory");
    linkDirOption.setShortMatchRule(SysCmdLine::Option::ShortMatchAll);
    linkDirOption.addArgument(Argument("dir"));
    linkDirOption.setUnlimitedOncurrence();

    Option includeDirOption("-I", "Add include directory");
    includeDirOption.setShortMatchRule(SysCmdLine::Option::ShortMatchAll);
    includeDirOption.addArgument(Argument("dir"));
    includeDirOption.setUnlimitedOncurrence();

    Command rootCommand("gcc", "GNU C/C++ Compiler");
    rootCommand.addOptions({
        linkDirOption,
        includeDirOption,
    });
    rootCommand.addVersionOption("0.0.0.1");
    rootCommand.addHelpOption(true, true);
    rootCommand.setHandler([](const ParseResult &result) {
        u8printf("Link directories:\n");
        for (const auto &item : result.valuesForOption("-L"))
            u8printf("    %s\n", item.toString().data());
        u8printf("Include directories:\n");
        for (const auto &item : result.valuesForOption("-I"))
            u8printf("    %s\n", item.toString().data());
        return 0;
    });

    SYSCMDLINE_ASSERT_COMMAND(rootCommand);

    Parser parser(rootCommand);
    return parser.invoke(SysCmdLine::commandLineArguments());
}