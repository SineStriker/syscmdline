#include <iostream>
#include <filesystem>

#include <syscmdline/parser.h>
#include <syscmdline/system.h>
#include <syscmdline/strings.h>

int main(int /* argc */, char * /* argv */[]) {
    using SysCmdLine::Argument;
    using SysCmdLine::Command;
    using SysCmdLine::Option;
    using SysCmdLine::Parser;
    using SysCmdLine::ParseResult;

    Argument weekdayArg("weekday", "Weekday");
    weekdayArg.setExpectedValues({
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Sunday",
    });

    Argument eventArg("event", "Event to do");
    eventArg.setDefaultValue("football");

    Command rootCommand("work", "What to do on which day?");
    rootCommand.setArguments({weekdayArg, eventArg});
    rootCommand.addHelpOption();
    rootCommand.setHandler([](const ParseResult &result) {
        std::cout << result.valueForArgument("weekday").toString() << std::endl;
        std::cout << result.valueForArgument("event").toString() << std::endl;
        return 0;
    });

    Parser parser(rootCommand);
    parser.setDisplayOptions(Parser::ShowArgumentDefaultValue | Parser::ShowArgumentExpectedValues);
    return parser.invoke(SysCmdLine::commandLineArguments());
}