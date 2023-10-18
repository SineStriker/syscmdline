#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#include <syscmdline/command.h>

namespace SysCmdLine {

    class ParserPrivate;

    class SYSCMDLINE_EXPORT Parser {
    public:
        Parser();
        Parser(const Command &rootCommand);
        ~Parser();

        Parser(const Parser &other) = delete;
        Parser &operator=(const Parser &other) = delete;

    public:
        enum Side {
            Top,
            Bottom,
        };

        enum Error {
            NoError,
            UnknownOption,
            UnknownCommand,
            MissingOptionArgument,
            MissingCommandArgument,
            TooManyArguments,
            InvalidArgumentValue,
        };

        const Command &rootCommand() const;
        void setRootCommand(const Command &rootCommand);

        std::string text(Side side) const;
        void setText(Side side, const std::string &text);

        bool parse(const std::vector<std::string> &args);
        int invoke(const std::vector<std::string> &args, int errorCode = -1);

        bool parsed() const;
        Error error() const;
        std::string errorText() const;

        Command targetCommand() const;
        std::vector<Option> targetGlobalOptions() const;
        std::vector<int> targetStack() const;

        void showError() const;
        void showHelpText() const;
        void showErrorAndHelpText(const std::string &message) const;
        void showWarningAndHelpText(const std::string &message) const;

    public:
        inline std::string value(const Argument &arg) const;
        std::string value(const std::string &argName) const;

        inline int count(const Option &opt) const;
        int count(const std::string &optName) const;

        inline std::string value(const Option &opt, const Argument &arg, int count = 0);
        std::string value(const std::string &optName, const std::string &argName, int count = 0);

        std::vector<std::string> effectiveOptions() const;
        std::vector<std::string> effectiveArguments() const;

        bool isHelpSet() const;
        bool isVersionSet() const;

        bool isResultNull() const;

    protected:
        ParserPrivate *d;
    };

    inline std::string Parser::value(const Argument &arg) const {
        return value(arg.name());
    }

    inline int Parser::count(const Option &opt) const {
        return count(opt.name());
    }

    inline std::string Parser::value(const Option &opt, const Argument &arg, int count) {
        return value(opt.name(), arg.name(), count);
    }

}

#endif // PARSER_H
