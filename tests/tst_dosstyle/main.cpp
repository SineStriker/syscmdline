#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

static const char INDENT[] = "    ";

static int routine(const ParseResult &result) {
    auto fileValues = Value::toStringList(result.valuesForArgument("files"));
    std::cout << "Files to be removed:" << std::endl;
    for (const auto &item : std::as_const(fileValues)) {
        std::cout << INDENT << item << std::endl;
    }

    bool prompt = result.optionIsSet("/P");
    bool force = result.optionIsSet("/F");
    bool subdir = result.optionIsSet("/S");
    bool quiet = result.optionIsSet("/Q");

    std::cout << "Dir: " << result.valueForArgument("dir").toString() << std::endl;

    std::cout << "Modes: " << std::endl;
    if (prompt) {
        std::cout << INDENT << "prompt" << std::endl;
    }
    if (force) {
        std::cout << INDENT << "force" << std::endl;
    }
    if (subdir) {
        std::cout << INDENT << "subdir" << std::endl;
    }
    if (quiet) {
        std::cout << INDENT << "quiet" << std::endl;
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


    Argument urlArg("url", "url");
    urlArg.setDisplayName("url");

    Argument fileArg("files", "File names to be removed");
    fileArg.setDisplayName("files");
    fileArg.setMultiValueEnabled(true);

    Argument dirArg("dir", "Directory destination");
    dirArg.setDisplayName("dir");

    Command rootCommand("del");
    rootCommand.addArgument(urlArg);
    rootCommand.addArgument(fileArg);
    rootCommand.addArgument(dirArg);
    rootCommand.setOptions({
        promptOption,
        forceOption,
        subdirOption,
        quietOption,
    });
    rootCommand.addHelpOption(false, false, {"/?"});
    rootCommand.setHandler(routine);

    Parser parser(rootCommand);
    parser.setDisplayOptions(Parser::ShowOptionalOptionsOnUsage);
    return parser.invoke(commandLineArguments(), -1, Parser::IgnoreOptionCase);
}