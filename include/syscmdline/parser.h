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
            InvalidOptionPosition,
            MissingRequiredOption,
            OptionOccurTooMuch,
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
        std::string correctionText() const;

        Command targetCommand() const;
        std::vector<Option> targetGlobalOptions() const;
        std::vector<int> targetStack() const;

        void showError() const;
        void showHelpText() const;
        void showErrorAndHelpText(const std::string &message) const;
        void showWarningAndHelpText(const std::string &message) const;

    public:
        inline std::string valueForArgument(const Argument &arg) const;
        std::string valueForArgument(const std::string &argName) const;

        inline int optionCount(const Option &opt) const;
        int optionCount(const std::string &optName) const;

        inline bool optionIsSet(const Option &opt) const;
        inline bool optionIsSet(const std::string &optName) const;

        inline std::string valueForOption(const Option &opt, const Argument &arg = {},
                                          int count = 0) const;
        std::string valueForOption(const std::string &optName, const std::string &argName = {},
                                   int count = 0) const;

        std::vector<std::string> effectiveOptions() const;
        std::vector<std::string> effectiveArguments() const;

        bool isHelpSet() const;
        bool isVersionSet() const;

        bool isResultNull() const;

    protected:
        ParserPrivate *d;
    };

    inline std::string Parser::valueForArgument(const Argument &arg) const {
        return valueForArgument(arg.name());
    }

    inline int Parser::optionCount(const Option &opt) const {
        return optionCount(opt.name());
    }

    bool Parser::optionIsSet(const Option &opt) const {
        return optionCount(opt) > 0;
    }

    bool Parser::optionIsSet(const std::string &optName) const {
        return optionCount(optName) > 0;
    }

    inline std::string Parser::valueForOption(const Option &opt, const Argument &arg,
                                              int count) const {
        return valueForOption(opt.name(), arg.name(), count);
    }

}

#endif // PARSER_H
