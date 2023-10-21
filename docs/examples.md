# Examples

## Basic

```c++
int main(int /* argc */, char * /* argv */ []) {
    using SysCmdLine::Argument;
    using SysCmdLine::Option;
    using SysCmdLine::Command;
    using SysCmdLine::Parser;
    using SysCmdLine::ParseResult;

    Argument srcArg("src", "Source");
    Argument destArg("dest", "Destination");

    Command rootCommand("mv", "Move file or directory.");
    rootCommand.setArguments({srcArg, destArg});
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

## Global Option