/****************************************************************************
 *
 * MIT License
 *
 * Copyright (c) 2023 SineStriker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ****************************************************************************/

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

        // Get values of multi-value argument at the option's N-th occurrence
        inline std::vector<Value> valuesForArgument(const Argument &arg, int index = 0) const;
        inline std::vector<Value> valuesForArgument(const std::string &name, int index = 0) const;
        std::vector<Value> valuesForArgument(int argIndex = 0, int index = 0) const;

        // Get value of single-value argument at the option's N-th occurrence or its default value
        inline Value valueForArgument(const Argument &arg, int index = 0) const;
        inline Value valueForArgument(const std::string &name, int index = 0) const;
        Value valueForArgument(int argIndex = 0, int index = 0) const;

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
        int indexOfOption(const std::string &token) const;

        void showError() const;
        void showHelpText() const;
        void showMessage(const std::string &info, const std::string &warning,
                         const std::string &error) const;

    public:
        bool isHelpSet() const;
        bool isVersionSet() const;

        // Get values of multi-value argument
        inline std::vector<Value> valuesForArgument(const Argument &arg) const;
        inline std::vector<Value> valuesForArgument(const std::string &name) const;
        std::vector<Value> valuesForArgument(int index) const;

        // Get value of single-value argument or its default value
        inline Value valueForArgument(const Argument &arg) const;
        inline Value valueForArgument(const std::string &name) const;
        Value valueForArgument(int index) const;

        inline bool optionIsSet(const Option &option) const;
        inline bool optionIsSet(const std::string &token) const;
        inline bool optionIsSet(int index) const;

        // Get values of single argument option which occurs multiple times
        inline std::vector<Value> valuesForOption(const Option &option) const;
        inline std::vector<Value> valuesForOption(const std::string &token) const;
        inline std::vector<Value> valuesForOption(int index) const;

        // Get value of single argument option at its first occurrence or its default value
        inline Value valueForOption(const Option &option) const;
        inline Value valueForOption(const std::string &token) const;
        inline Value valueForOption(int index) const;

        // Detailed result for an option
        inline OptionResult resultForOption(const Option &option) const;
        inline OptionResult resultForOption(const std::string &token) const;
        OptionResult resultForOption(int index) const;

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
        return resultForOption(indexOfOption(option.token())).count() > 0;
    }

    inline bool ParseResult::optionIsSet(const std::string &token) const {
        return resultForOption(indexOfOption(token)).count() > 0;
    }

    bool ParseResult::optionIsSet(int index) const {
        return resultForOption(index).count() > 0;
    }

    inline std::vector<Value> ParseResult::valuesForOption(const Option &option) const {
        return valuesForOption(indexOfOption(option.token()));
    }

    inline std::vector<Value> ParseResult::valuesForOption(const std::string &token) const {
        return valuesForOption(indexOfOption(token));
    }

    inline std::vector<Value> ParseResult::valuesForOption(int index) const {
        OptionResult optionResult = resultForOption(index);
        if (!optionResult.isValid())
            return {};

        std::vector<Value> values;
        values.reserve(optionResult.count());
        for (int i = 0; i < optionResult.count(); ++i) {
            values.emplace_back(optionResult.valueForArgument(0, i));
        }
        return values;
    }

    inline Value ParseResult::valueForOption(const Option &option) const {
        return resultForOption(indexOfOption(option.token())).valueForArgument(0);
    }

    inline Value ParseResult::valueForOption(const std::string &token) const {
        return resultForOption(indexOfOption(token)).valueForArgument(0);
    }

    Value ParseResult::valueForOption(int index) const {
        return resultForOption(index).valueForArgument(0);
    }

    inline OptionResult ParseResult::resultForOption(const Option &option) const {
        return resultForOption(indexOfOption(option.token()));
    }

    OptionResult ParseResult::resultForOption(const std::string &token) const {
        return resultForOption(indexOfOption(token));
    }

}

#endif // PARSERESULT_H
