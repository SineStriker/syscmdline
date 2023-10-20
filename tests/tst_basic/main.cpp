#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

int main(int argc, char *argv[]) {
    SYSCMDLINE_UNUSED(argc);
    SYSCMDLINE_UNUSED(argv);

    Command cloneCommand("clone", "Clone a repository into a new directory");
    cloneCommand.addArgument(Argument("url", "Repository url"));
    cloneCommand.addHelpOption();
    cloneCommand.setHandler([](const ParseResult &result) -> int {
        u8printf("clone\n");
        return 0;
    });

    Command commitCommand("commit", "Record changes to the repository");
    commitCommand.setHandler([](const ParseResult &result) -> int {
        u8printf("commit\n");
        return 0;
    });

    Command mergeCommand("merge", "Join two or more development histories together");
    mergeCommand.setHandler([](const ParseResult &result) -> int {
        u8printf("merge\n");
        return 0;
    });

    Argument arg1("wangwenx190", "You know who");
    Argument arg2("name", "Your name", false);
    Argument arg3("age", "Your age", false);
    arg1.setExpectedValues({"genius"});

    Option opt1("link", "link libraries", {"-l", "--link"}, true,
                {
                    Argument("lib", "library"),
                });

    Command rootCommand("git", "Git is a distributed version management system.");
    rootCommand.setCommands({cloneCommand, commitCommand, mergeCommand});
    rootCommand.setArguments({arg1, arg2, arg3});
    rootCommand.addOption(opt1);
    rootCommand.addVersionOption("0.0.1.1");
    rootCommand.addHelpOption(true, true);
    rootCommand.setHandler([](const ParseResult &result) -> int {
        u8printf("git\n");
        return 0;
    });

    CommandCatalogue cc;
    cc.addCommandCatalogue("Special Commands", {"clone"});
    rootCommand.setCatalogue(cc);

    Parser parser(rootCommand);
    parser.setText(Parser::Top, "Git Version 0.0.1.1");
    parser.setText(Parser::Bottom, "Checkout https://github.com/git/git for more information.");

    return parser.invoke(commandLineArguments());
}