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

#ifndef PARSER_H
#define PARSER_H

#include <syscmdline/command.h>
#include <syscmdline/helplayout.h>

namespace SysCmdLine {

    class ParseResultData;

    class SYSCMDLINE_EXPORT ParseResult {
        SYSCMDLINE_DECL_DATA(ParseResult)
    public:
        ParseResult();
        ~ParseResult();

        ParseResult(const ParseResult &other);
        ParseResult(ParseResult &&other) noexcept;
        ParseResult &operator=(const ParseResult &other);
        ParseResult &operator=(ParseResult &&other) noexcept;

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
        void showErrorAndHelpText(const std::string &message) const;
        void showWarningAndHelpText(const std::string &message) const;

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
        bool isResultNull() const;

    protected:
        SharedDataPointer<ParseResultData> d_ptr;

        ParseResult(ParseResultData *d);

        friend class Parser;
        friend class ParserData;
    };

    inline bool ParseResult::isValid() const {
        return d_ptr.data() != nullptr;
    }

    inline Value ParseResult::valueForArgument(const Argument &arg) const {
        return valueForArgument(arg.name());
    }

    inline std::vector<Value> ParseResult::valuesForArgument(const Argument &arg) const {
        return valuesForArgument(arg.name());
    }

    inline int ParseResult::optionCount(const Option &opt) const {
        return optionCount(opt.name());
    }

    bool ParseResult::optionIsSet(const Option &opt) const {
        return optionCount(opt) > 0;
    }

    bool ParseResult::optionIsSet(const std::string &optName) const {
        return optionCount(optName) > 0;
    }

    inline Value ParseResult::valueForOption(const Option &opt, const Argument &arg,
                                             int count) const {
        return valueForOption(opt.name(), arg.name(), count);
    }

    class ParserData;

    class SYSCMDLINE_EXPORT Parser {
        SYSCMDLINE_DECL_DATA(Parser)
    public:
        Parser();
        Parser(const Command &rootCommand);
        ~Parser();

        Parser(const Parser &other);
        Parser(Parser &&other) noexcept;
        Parser &operator=(const Parser &other);
        Parser &operator=(Parser &&other) noexcept;

    public:
        enum Position {
            Prologue,
            Epilogue,
        };

        enum ParseOption {
            Standard = 0,
            IgnoreCommandCase = 0x1,
            IgnoreOptionCase = 0x2,
            AllowUnixGroupFlags = 0x4,
            AllowDosKeyValueOptions = 0x8,
            DontAllowUnixKeyValueOptions = 0x10,
        };

        enum DisplayOption {
            Normal = 0,
            DontShowHelpOnError = 0x1,
            SkipCorrection = 0x2,
            DontShowRequiredOptionsOnUsage = 0x4,
            ShowOptionalOptionsOnUsage = 0x8,
            ShowOptionsBehindArguments = 0x10,
            ShowArgumentDefaultValue = 0x20,
            ShowArgumentExpectedValues = 0x40,
            ShowArgumentIsRequired = 0x80,
            ShowOptionIsRequired = 0x100,
        };

        std::string intro(Position pos) const;
        void setIntro(Position pos, const std::string &text);

        int displayOptions() const;
        void setDisplayOptions(int displayOptions);

        HelpLayout helpLayout() const;
        void setHelpLayout(const HelpLayout &helpLayout);

        Command rootCommand() const;
        void setRootCommand(const Command &rootCommand);

        ParseResult parse(const std::vector<std::string> &args, int parseOptions = Standard);
        inline int invoke(const std::vector<std::string> &args, int errCode = -1,
                          int parseOptions = Standard);

    protected:
        SharedDataPointer<ParserData> d_ptr;

        friend class ParserData;
    };

    inline int Parser::invoke(const std::vector<std::string> &args, int errCode, int parseOptions) {
        return parse(args, parseOptions).invoke(errCode);
    }

}

#endif // PARSER_H
