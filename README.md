# SysCmdLine

C++ Advanced Command Line Parser.

Inspired by Qt `QCommandLineParser` and C# `System.CommandLine`.

## Introduction

To be in line with the principle of "don't learn anything new if you don't need to", on the one hand, `syscmdline` contains as many common features as possible, on the other hand, it's simple enough to be easy to understand.

Therefore, the project is designed to be configurable, but it's not intended to be as complex as a framework.

## Features

+ Support sub-commands
+ Support case-insensitive parsing
+ Support global options
+ Support mutually exclusive options
+ Support short options and group flags
+ Support help text customization
+ Support localization
+ Simple tips for typo correction
+ Highly configurable
+ Interface friendly

<!-- ## Concepts

If you are confused about some of the concepts of command line programs, you can learn the following, which will help you use this project.

See [Concepts](docs/concepts.md) to learn more. -->


## Help Text

The help text is highly configurable, here we show some simple ones.

### Unix Style
```
Description:
    Move source(s) to destination directory.

Usage:
    mv <src>... <dest> [options]

Arguments:
    src     Source(s)
    dest    Destination directory

Options:
    -v, --version    Show version information
    -h, --help       Show help information
```
### Dos Style
```
Description:
    Deletes one or more files.

Usage:
    del files... [/P] [/F] [/S] [/Q] [/?]

Arguments:
    files    Specifies a list of one or more files or directories.

Options:
    /P       Prompts for confirmation before deleting the specified file.
    /F       Forces deletion of read-only files.
    /S       Deletes specified files from the current directory and all subdirectories.
    /Q       Specifies quiet mode. You are not prompted for delete confirmation.
    /?       Show help information.
```

## Quick Start

See [Examples](docs/examples.md) to learn more.

### Build & Install

```sh
cmake -B build -G Ninja
cmake --build build --target all
cmake --build build --target install
```

### Integrate

```cmake
find_package(syscmdline REQUIRED)
target_link_libraries(my_project PRIVATE syscmdline::syscmdline)
```

## Notice

+ C++ Standard
    + You need a C++ compiler which supports C++20 to build the library. The interface is compatible with C++11.

+ Minimize Size
    + In order to achieve more functionalities, this project contains a large amount of codes so that the binary size may be relatively large compared with other libraries. Therefore, this implementation uses STL templates as little as possible.
    + It's suggested to enable size optimizing option for your compiler when building executables.

## Thanks

### Other Projects

+ https://github.com/qt/qtbase
+ https://github.com/dotnet/command-line-api
<!-- + https://github.com/tanakh/cmdline
+ https://github.com/p-ranav/argparse
+ https://github.com/CLIUtils/CLI11 -->

### Contributors

+ [wangwenx190](https://github.com/wangwenx190)
+ [JobSecond](https://github.com/JobSecond)

## License

This project is licensed under the MIT License.