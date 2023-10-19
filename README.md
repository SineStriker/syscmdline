# SysCmdLine

Command Line Argument Parser for Mordern C++.

Inspired by Qt `QCommandLineParser` and C# `Systeme.CommandLine`.

## Highlights

+ Support sub-commands
+ Support case-insensitive parsing
+ Highly customizable

## Integrate

```cmake
find_package(syscmdline REQUIRED)
target_link_libraries(my_project PRIVATE syscmdline::syscmdline)
```