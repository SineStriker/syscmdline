#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include <syscmdline/command.h>

namespace SysCmdLine {

    enum ParseErrorType {
        UnknownCommand,
        UnknownOption,
    };

    template <class T>
    class ParseError {
    public:

    };

    template <class T>
    class Parser {
    public:
    };

}

#endif // PARSER_H
