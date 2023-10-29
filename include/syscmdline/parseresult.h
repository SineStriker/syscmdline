#ifndef PARSERESULT_H
#define PARSERESULT_H

#include <syscmdline/command.h>

namespace SysCmdLine {

    class Parser;

    class ParseResultPrivate;

    class SYSCMDLINE_EXPORT OptionResult {
    public:
        inline OptionResult();

    public:
        inline bool isValid() const;

        Option option() const;
        int argumentIndex(const std::string &argName) const;
        int count() const;

        inline std::vector<Value> valuesForArgument(const Argument &arg, int index = 0) const;
        inline std::vector<Value> valuesForArgument(const std::string &name, int index = 0) const;
        std::vector<Value> valuesForArgument(int argIndex, int index = 0) const;

        inline Value valueForArgument(const Argument &arg, int index = 0) const;
        inline Value valueForArgument(const std::string &name, int index = 0) const;
        Value valueForArgument(int argIndex, int index = 0) const;

    private:
        inline OptionResult(const void *data);
        const void *data;

        friend class ParseResult;
    };

    OptionResult::OptionResult() : data(nullptr) {
    }

    inline bool OptionResult::isValid() const {
        return data != nullptr;
    }

    inline std::vector<Value> OptionResult::valuesForArgument(const Argument &arg,
                                                              int index) const {
        return valuesForArgument(argumentIndex(arg.name()), index);
    }

    inline std::vector<Value> OptionResult::valuesForArgument(const std::string &name,
                                                              int index) const {
        return valuesForArgument(argumentIndex(name), index);
    }

    inline Value OptionResult::valueForArgument(const Argument &arg, int index) const {
        return valueForArgument(argumentIndex(arg.name()), index);
    }

    inline Value OptionResult::valueForArgument(const std::string &name, int index) const {
        return valueForArgument(argumentIndex(name), index);
    }

    inline OptionResult::OptionResult(const void *data) : data(data) {
    }

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
        int indexOfArgument(const std::string &argName) const;

        void showError() const;
        void showHelpText() const;
        void showMessage(const std::string &info, const std::string &warning,
                         const std::string &error) const;

    public:
        bool isHelpSet() const;
        bool isVersionSet() const;

        inline std::vector<Value> valuesForArgument(const Argument &arg) const;
        inline std::vector<Value> valuesForArgument(const std::string &name) const;
        std::vector<Value> valuesForArgument(int index) const;

        inline Value valueForArgument(const Argument &arg) const;
        inline Value valueForArgument(const std::string &name) const;
        Value valueForArgument(int index) const;

        inline bool optionIsSet(const Option &option) const;
        bool optionIsSet(const std::string &token) const;

        inline std::vector<Value> valuesForOption(const Option &option) const;
        std::vector<Value> valuesForOption(const std::string &token) const;

        inline Value valueForOption(const Option &option) const;
        Value valueForOption(const std::string &token) const;

        inline OptionResult resultForOption(const Option &option) const;
        OptionResult resultForOption(const std::string &token) const;

    protected:
        ParseResult(ParseResultPrivate *d);

        friend class Parser;
    };

    inline bool ParseResult::isValid() const {
        return d_ptr != nullptr;
    }

    inline std::vector<Value> ParseResult::valuesForArgument(const Argument &arg) const {
        return valuesForArgument(indexOfArgument(arg.name()));
    }

    inline std::vector<Value> ParseResult::valuesForArgument(const std::string &name) const {
        return valuesForArgument(indexOfArgument(name));
    }

    inline Value ParseResult::valueForArgument(const Argument &arg) const {
        return valueForArgument(indexOfArgument(arg.name()));
    }

    inline Value ParseResult::valueForArgument(const std::string &name) const {
        return valueForArgument(indexOfArgument(name));
    }

    inline bool ParseResult::optionIsSet(const Option &option) const {
        return optionIsSet(option.token());
    }

    inline std::vector<Value> ParseResult::valuesForOption(const Option &option) const {
        return valuesForOption(option.token());
    }

    Value ParseResult::valueForOption(const Option &option) const {
        return valueForOption(option.token());
    }

    inline OptionResult ParseResult::resultForOption(const Option &option) const {
        return resultForOption(option.token());
    }

}

#endif // PARSERESULT_H
