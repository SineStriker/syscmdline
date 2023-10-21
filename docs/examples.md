# Examples

+ Basic
    + Positional Arguments
    + Optinal Argument And Restricted Argument
    + Argument Validator
    + Options
    + Commands
    + Sub-commands

## Basic

### Positional Arguments

```c++
using SysCmdLine::Argument;
using SysCmdLine::Option;
using SysCmdLine::Command;
using SysCmdLine::Parser;
using SysCmdLine::ParseResult;

int main(int /* argc */, char * /* argv */ []) {
    Argument srcArg("src", "Source");
    Argument destArg("dest", "Destination");

    Command rootCommand("mv", "Move file or directory.");
    rootCommand.setArguments({srcArg, destArg});
    rootCommand.addVersionOption("0.0.1.1");
    rootCommand.addHelpOption();
    rootCommand.setHandler([](const ParseResult &result) {
        std::cout << result.valueForArgument("src").toString() << std::endl;
        std::cout << result.valueForArgument("dest").toString() << std::endl;
        return 0;
    });

    Parser parser(rootCommand);
    return parser.invoke(SysCmdLine::commandLineArguments());
}
```

+ In this case, we simply build a `mv` command requiring a `src` and a `dest` arguments. We also add the default version and help option to the program.

+ If the arguments are valid, the program will print `src` and `dest` value.

+ On Windows, parameters from `main` function are ANSI strings which doesn't support some Unicode characters. `syscmdline` uses UTF-8 encoding to privide internal texts and print to the console. Therefore, it is recommended to use `SysCmdLine::commandLineArguments()` rather than `argc` and `argv`, this function provides a cross-platform aproach to query command line arguments and return the strings encoded in UTF-8.

```sh
Description:
    Move file or directory.

Usage:
    mv <src> <dest> [options]

Arguments:
    <src>     Source
    <dest>    Destination

Options:
    -v, --version    Show version information
    -h, --help       Show help information
```

### Optinal Argument And Restricted Argument

```c++
int main(int /* argc */, char * /* argv */ []) {
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
    eventArg.setRequired(false);

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
```
+ In this case, we add a restricted argument and another optional argument with default value.

+ If an argument has an expected value list, only symbols listed in it will be regarded as valid argument.

+ We also tell the parser to show default value and expected values on help text which would be hided if the flags weren't set.

```sh
Description:
    What to do on which day?

Usage:
    work <weekday> [<event>] [options]

Arguments:
    <weekday>    Weekday [Expected: "Monday", "Tuesday", "Wednesday", ...]
    <event>      Event to do [Default: football]

Options:
    -h, --help    Show help information
```

### Argument Validator

TODO

## Global Option

TODO