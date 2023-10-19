#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

static int routine(const Parser &parser) {
    auto fileValues = Value::toStringList(parser.valuesForArgument("files"));
    std::cout << "Files to be removed:" << std::endl;
    for (const auto &item : std::as_const(fileValues)) {
        std::cout << "    " << item << std::endl;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    Option promptOption("/P", "Prompts for confirmation before deleting the specified file.");
    Option forceOption("/F", "Forces deletion of read-only files.");
    Option subdirOption(
        "/S", "Deletes specified files from the current directory and all subdirectories. Displays "
              "the names \nof the files as they are being deleted.");
    Option quietOption("/Q", "Specifies quiet mode. You are not prompted for delete confirmation.");

    Argument fileArg("files", "File names to be removed");
    fileArg.setDisplayName("files");

    Command rootCommand("del");
    rootCommand.addArgument(fileArg);
    rootCommand.setOptions({
        promptOption,
        forceOption,
        subdirOption,
        quietOption,
    });
    rootCommand.addHelpOption(false, false, {"/?"});
    rootCommand.setMultipleArgumentsEnabled(true);
    rootCommand.setHandler(routine);

    Parser parser(rootCommand);
    return parser.invoke(commandLineArguments());
}