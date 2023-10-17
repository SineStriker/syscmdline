#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include <syscmdline/command.h>

namespace SysCmdLine {

    class ParseResult {
    public:
        ParseResult();
        ~ParseResult();

        bool isSet(const ParseResult &res, const Option &opt) const;
        bool isSet(const ParseResult &res, const Argument &arg) const;
        bool isSet(const ParseResult &res, const Option &opt, const Argument &arg);
        std::string value(const ParseResult &res, const Argument &arg) const;
        std::string value(const ParseResult &res, const Option &opt, const Argument &arg);
    };

    class Parser {
    public:
        Parser();
        ~Parser();

        void parse();

        std::vector<int> commandStack() const;
        const Command &targetCommand() const;

    protected:
    };

}

#endif // PARSER_H
