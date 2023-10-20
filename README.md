# SysCmdLine

Command Line Parser for Mordern C++.

Inspired by Qt `QCommandLineParser` and C# `Systeme.CommandLine`.

## Features

+ Support sub-commands
+ Support case-insensitive parsing
+ Support mutually exclusive options
+ Support short options and flags
+ Simple tips for typo correction
+ Highly configurable
+ Interface friendly

## Integrate

```cmake
find_package(syscmdline REQUIRED)
target_link_libraries(my_project PRIVATE syscmdline::syscmdline)
```