# SysCmdLine

Command Line Parser for Mordern C++.

Inspired by Qt `QCommandLineParser` and C# `System.CommandLine`.

## Features

+ Support sub-commands
+ Support case-insensitive parsing
+ Support global options
+ Support mutually exclusive options
+ Support short options and flags
+ Support localization
+ Simple tips for typo correction
+ Interface friendly
+ Highly configurable

## Integrate

```cmake
find_package(syscmdline REQUIRED)
target_link_libraries(my_project PRIVATE syscmdline::syscmdline)
```

## Concepts

### Command

+ **Command** is the beginning symbol used to determine the behavior of a set of command-line arguments.

+ The commands of a CLI program are usually in the form of a tree structure, and the program itself is the root command. Take `git` as an example, `git` is the root command and `merge`/`commit`/... are the sub-commands.

+ However, most commands doesn't have sub-commands because the root command is sufficient in most scenarios.

+ Command line parameters must start with the root command and list the subcommands in sequence along the path to a certain leaf command. When the target sub-command is listed, all subsequent symbols are regarded as the arguments or options of this sub-command.

### Argument

+ **Argument** is the positional argument, which belongs to a command or an option.

+ The positional arguments are parsed according to the order in which they are specified. Take `mv` as an example, `mv <src> <dest>` shows that it has two positional arguments and the first will be recognized as source and the second as destination.

### Option

+ **Option**, usually a word or letter beginning with the `-` or `/` sign. (`-` is the Unix style and `/` is the Dos style)

+ Options can have none or a fixed number of positional arguments which are recommended to be non-optional.

## Examples

+ Global option
+ Multi-value argument
+ Short option and key-value pair option
+ Group flags
+ Prior option

See [Examples](docs/examples.md) to learn more.

## Thanks

### Other Projects

+ https://github.com/qt/qtbase
+ https://github.com/dotnet/command-line-api
+ https://github.com/tanakh/cmdline
+ https://github.com/p-ranav/argparse
+ https://github.com/CLIUtils/CLI11

### Contributors

+ [wangwenx190](https://github.com/wangwenx190)