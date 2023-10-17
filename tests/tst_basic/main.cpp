#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    Argument urlArg("url", "1");

    Command commitCommand("commit", "Record changes to the repository");
    commitCommand.addHelpOption();

    Command mergeCommand("merge", "Join two or more development histories together");
    mergeCommand.addHelpOption();

    Command rootCommand("git", "Git is a distributed version management system.");
    rootCommand.setCommands({commitCommand, mergeCommand});
    rootCommand.setArguments({urlArg});
    rootCommand.addVersionOption("0.0.1.1");
    rootCommand.addHelpOption();

    Parser parser(rootCommand);
    parser.setText(Parser::Top, "Git Version 0.0.1.1");
    parser.setText(Parser::Bottom, "https://github.com/git/git");

    parser.parse({
        "git",
        "merge",
        "--help"
    });
    if (parser.error() == Parser::NoError) {
        u8printf("cmd: %s\n", parser.targetCommand()->name().data());
        parser.invoke();
    }

    return 0;
}