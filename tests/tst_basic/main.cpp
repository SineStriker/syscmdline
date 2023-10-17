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
    Command mergeCommand("merge", "Join two or more development histories together");

    Command rootCommand("git", "Git is a distributed version management system.");
    rootCommand.setCommands({commitCommand, mergeCommand});
    rootCommand.setArguments({urlArg});
    rootCommand.addVersionOption("0.0.1.1");
    rootCommand.addHelpOption();

    u8printf("-----------------------------------\n");
    u8printf("%s", rootCommand.helpText().data());
    u8printf("-----------------------------------\n");
    return 0;
}