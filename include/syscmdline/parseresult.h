#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <syscmdline/command.h>

namespace SysCmdLine {

    class Parser;

    class ParseResultPrivate;

    class SYSCMDLINE_EXPORT ParseResult : public SharedBase {
        SYSCMDLINE_DECL_PRIVATE(ParseResult)
    public:
        ParseResult();
        ~ParseResult();

        inline bool isValid() const;

    public:
        Command rootCommand() const;
        const std::vector<std::string> &arguments() const;

        int invoke(int errCode = -1) const;
        int dispatch() const;

    public:
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
            ArgumentTypeMismatch,
            ArgumentValidateFailed,
            MutuallyExclusiveOptions,
            PriorOptionWithArguments,
            PriorOptionWithOptions,
        };

        Error error() const;
        std::string errorText() const;
        std::string correctionText() const;
        std::string cancellationToken() const;

        Command command() const;
        std::vector<Option> globalOptions() const;
        std::vector<int> commandIndexStack() const;

        void showError() const;
        void showHelpText() const;
        void showMessage(const std::string &info, const std::string &warning,
                         const std::string &error) const;

    public:
        inline Value valueForArgument(const Argument &arg) const;
        Value valueForArgument(const std::string &argName) const;
        inline std::vector<Value> valuesForArgument(const Argument &arg) const;
        std::vector<Value> valuesForArgument(const std::string &argName) const;

        inline int optionCount(const Option &opt) const;
        int optionCount(const std::string &optName) const;

        inline bool optionIsSet(const Option &opt) const;
        inline bool optionIsSet(const std::string &optName) const;

        inline Value valueForOption(const Option &opt, const Argument &arg, int count = 0) const;
        Value valueForOption(const std::string &optName, const std::string &argName,
                             int count = 0) const;
        Value valueForOption(const std::string &optName, int argIndex = 0, int count = 0) const;

        std::vector<std::string> effectiveOptions() const;
        std::vector<std::string> effectiveArguments() const;

        bool isHelpSet() const;
        bool isVersionSet() const;

    protected:
        ParseResult(ParseResultPrivate *d);

        friend class Parser;
    };

    inline bool ParseResult::isValid() const {
        return d_ptr != nullptr;
    }

}

#endif // PARSERESULT_H
