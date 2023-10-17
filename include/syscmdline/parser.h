#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include <syscmdline/command.h>

namespace SysCmdLine {

    class ParserPrivate;

    class Parser {
    public:
        Parser();
        Parser(const Command &rootCommand);
        ~Parser();

    public:
        enum Side {
            Top,
            Bottom,
        };

        enum Error {
            NoError,
            UnknownArgument,
            MissingRequiredArgument,
        };

        const Command &rootCommand() const;
        void setRootCommand(const Command &rootCommand);

        std::string text(Side side) const;
        void setText(Side side, const std::string &text);

        bool parse(const std::vector<std::string> &args);
        int invoke(const std::vector<std::string> &args);

        bool parsed() const;
        Error error() const;
        std::string errorText() const;

        const Command *targetCommand() const;
        const std::vector<const Option *> &targetGlobalOptions() const;
        std::vector<std::pair<int, std::string>> targetStack() const;

        void showHelpText() const;

    public:
        std::string value(const Argument &arg) const;
        int count(const Option &opt) const;
        std::string value(const Option &opt, const Argument &arg, int count = 0);

        bool isHelpSet() const;
        bool isVersionSet() const;

    protected:
        ParserPrivate *d;
    };

}

#endif // PARSER_H
