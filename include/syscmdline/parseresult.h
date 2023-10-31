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
        OptionResult();

    public:
        Option option() const;
        int argumentIndex(const std::string &name) const;
        int count() const;

        inline bool argumentIsSet(const Argument &arg, int n = 0) const;
        inline bool argumentIsSet(const std::string &name, int n = 0) const;
        inline bool argumentIsSet(int index, int n = 0) const;

        // Get values of multi-value argument at the option's N-th occurrence
        inline const std::vector<Value> &values(const Argument &arg, int n = 0) const;
        inline const std::vector<Value> &values(const std::string &name, int n = 0) const;
        const std::vector<Value> &values(int index = 0, int n = 0) const;

        // Get value of single-value argument at the option's N-th occurrence or its default value
        inline Value value(const Argument &arg, int n = 0) const;
        inline Value value(const std::string &name, int n = 0) const;
        Value value(int index = 0, int n = 0) const;

    private:
        inline OptionResult(const void *data);
        const void *data;

        friend class ParseResult;
    };

    inline bool OptionResult::argumentIsSet(const Argument &arg, int n) const {
        return !values(argumentIndex(arg.name()), n).empty();
    }
    inline bool OptionResult::argumentIsSet(const std::string &name, int n) const {
        return !values(argumentIndex(name), n).empty();
    }
    inline bool OptionResult::argumentIsSet(int index, int n) const {
        return !values(index, n).empty();
    }

    inline const std::vector<Value> &OptionResult::values(const Argument &arg, int n) const {
        return values(argumentIndex(arg.name()), n);
    }

    inline const std::vector<Value> &OptionResult::values(const std::string &name, int n) const {
        return values(argumentIndex(name), n);
    }

    inline Value OptionResult::value(const Argument &arg, int n) const {
        return value(argumentIndex(arg.name()), n);
    }

    inline Value OptionResult::value(const std::string &name, int n) const {
        return value(argumentIndex(name), n);
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
        int indexOfArgument(const std::string &name) const;
        int indexOfOption(const std::string &token) const;

        void showError() const;
        void showHelpText() const;
        void showMessage(const std::string &info, const std::string &warning,
                         const std::string &error) const;

    public:
        bool isHelpSet() const;
        bool isVersionSet() const;

        inline bool argumentIsSet(const Argument &arg) const;
        inline bool argumentIsSet(const std::string &name) const;
        inline bool argumentIsSet(int index) const;

        // Get values of multi-value argument
        inline const std::vector<Value> &valuesForArgument(const Argument &arg) const;
        inline const std::vector<Value> &valuesForArgument(const std::string &name) const;
        const std::vector<Value> &valuesForArgument(int index) const;

        // Get value of single-value argument or its default value
        inline Value valueForArgument(const Argument &arg) const;
        inline Value valueForArgument(const std::string &name) const;
        Value valueForArgument(int index) const;

        inline bool optionIsSet(const Option &option) const;
        inline bool optionIsSet(const std::string &token) const;
        inline bool optionIsSet(int index) const;

        // Get all values of the argument at `index` of the option
        inline std::vector<Value> valuesForOption(const Option &option, int index = 0) const;
        inline std::vector<Value> valuesForOption(const std::string &token, int index = 0) const;
        inline std::vector<Value> valuesForOption(int optionIndex, int index = 0) const;

        // Get value of single argument option at its first occurrence or its default value
        inline Value valueForOption(const Option &option) const;
        inline Value valueForOption(const std::string &token) const;
        inline Value valueForOption(int index) const;

        // Detailed result for an option
        inline OptionResult option(const Option &option) const;
        inline OptionResult option(const std::string &token) const;
        OptionResult option(int index) const;

    protected:
        ParseResult(ParseResultPrivate *d);

        friend class Parser;
    };

    inline bool ParseResult::isValid() const {
        return d_ptr != nullptr;
    }

    inline bool ParseResult::argumentIsSet(const Argument &arg) const {
        return argumentIsSet(indexOfArgument(arg.name()));
    }

    inline bool ParseResult::argumentIsSet(const std::string &name) const {
        return argumentIsSet(indexOfArgument(name));
    }

    inline bool ParseResult::argumentIsSet(int index) const {
        return !valuesForArgument(index).empty();
    }

    inline const std::vector<Value> &ParseResult::valuesForArgument(const Argument &arg) const {
        return valuesForArgument(indexOfArgument(arg.name()));
    }

    inline const std::vector<Value> &ParseResult::valuesForArgument(const std::string &name) const {
        return valuesForArgument(indexOfArgument(name));
    }

    inline Value ParseResult::valueForArgument(const Argument &arg) const {
        return valueForArgument(indexOfArgument(arg.name()));
    }

    inline Value ParseResult::valueForArgument(const std::string &name) const {
        return valueForArgument(indexOfArgument(name));
    }

    inline bool ParseResult::optionIsSet(const Option &option) const {
        return this->option(indexOfOption(option.token())).count() > 0;
    }

    inline bool ParseResult::optionIsSet(const std::string &token) const {
        return option(indexOfOption(token)).count() > 0;
    }

    bool ParseResult::optionIsSet(int index) const {
        return option(index).count() > 0;
    }

    inline std::vector<Value> ParseResult::valuesForOption(const Option &option, int index) const {
        return valuesForOption(indexOfOption(option.token()), index);
    }

    inline std::vector<Value> ParseResult::valuesForOption(const std::string &token,
                                                           int index) const {
        return valuesForOption(indexOfOption(token), index);
    }

    inline std::vector<Value> ParseResult::valuesForOption(int optionIndex, int index) const {
        OptionResult optionResult = option(optionIndex);
        std::vector<Value> allValues;
        for (int i = 0; i < optionResult.count(); ++i) {
            const auto &values = optionResult.values(index, i);
            allValues.insert(allValues.end(), values.begin(), values.end());
        }
        return allValues;
    }

    inline Value ParseResult::valueForOption(const Option &option) const {
        return this->option(indexOfOption(option.token())).value(0);
    }

    inline Value ParseResult::valueForOption(const std::string &token) const {
        return option(indexOfOption(token)).value(0);
    }

    inline Value ParseResult::valueForOption(int index) const {
        return option(index).value(0);
    }

    inline OptionResult ParseResult::option(const Option &option) const {
        return this->option(indexOfOption(option.token()));
    }

    inline OptionResult ParseResult::option(const std::string &token) const {
        return option(indexOfOption(token));
    }

}

#endif // PARSERESULT_H
