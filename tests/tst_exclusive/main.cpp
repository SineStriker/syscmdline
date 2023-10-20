#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>

using namespace SysCmdLine;

static int routine(const Parser &parser) {
    std::cout << "A: " << parser.optionIsSet("a") << std::endl;
    std::cout << "B: " << parser.optionIsSet("b") << std::endl;
    std::cout << "C: " << parser.optionIsSet("c") << std::endl;
    std::cout << "D: " << parser.optionIsSet("d") << std::endl;
    return 0;
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    Option option1("a", "A");
    option1.setShortMatchRule(Option::ShortMatchAll);
    option1.setRequired(true);

    Option option2("b", "B");
    option2.setShortMatchRule(Option::ShortMatchAll);
    option2.setRequired(true);

    Option option3("c", "C");
    option3.setShortMatchRule(Option::ShortMatchAll);
    option3.setRequired(true);

    Option option4("d", "D");
    option4.setShortMatchRule(Option::ShortMatchAll);

    Command rootCommand("test");
    rootCommand.setOptions({
        {option1, 1 },
        {option2, 1 },
        {option3, 1 },
        {option4, -1},
    });

    rootCommand.addVersionOption("0.0.0.1");
    rootCommand.addHelpOption(true);
    rootCommand.setHandler(routine);

    Parser parser(rootCommand);
    parser.setDisplayOptions(Parser::ShowOptionalOptionsOnUsage);
    return parser.invoke(commandLineArguments(), -1, Parser::ConsiderShortFlags);
}