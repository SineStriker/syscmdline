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

#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <functional>
#include <vector>

#include <syscmdline/symbol.h>
#include <syscmdline/value.h>

namespace SysCmdLine {
    
    class ArgumentHolder;
    class ArgumentHolderData;
    class Command;
    class CommandData;

    class ArgumentData;

    class SYSCMDLINE_EXPORT Argument : public Symbol {
        SYSCMDLINE_DECL_DATA(Argument)
    public:
        using Validator = std::function<bool /* result */ (
            const std::string & /* token */, Value * /* out */, std::string * /* errorMessage */)>;

        Argument();
        Argument(const std::string &name, const std::string &desc = {});
        Argument(const std::string &name, const std::string &desc, const Value &defaultValue,
                 bool required = true, const std::string &displayName = {},
                 bool multipleEnabled = false);
        Argument(const std::string &name, const std::string &desc,
                 const std::vector<Value> &expectedValues, const Value &defaultValue,
                 bool required = true, const std::string &displayName = {},
                 bool multipleEnabled = false);
        ~Argument();

        Argument(const Argument &other);
        Argument(Argument &&other) noexcept;
        Argument &operator=(const Argument &other);
        Argument &operator=(Argument &&other) noexcept;

        std::string displayedText() const;

        using Symbol::helpText;
        std::string helpText(HelpPosition pos, int displayOptions, void *extra) const override;

    public:
        const std::vector<Value> &expectedValues() const;
        void setExpectedValues(const std::vector<Value> &expectedValues);

        Value defaultValue() const;
        void setDefaultValue(const Value &defaultValue);

        std::string displayName() const;
        void setDisplayName(const std::string &displayName);

        bool isRequired() const;
        void setRequired(bool required);

        inline bool isOptional() const;
        inline void setOptional(bool optional);

        bool multiValueEnabled() const;
        void setMultiValueEnabled(bool on);

        Validator validator() const;
        void setValidator(const Validator &validator);

    protected:
        friend class ArgumentHolder;
        friend class ArgumentHolderData;
        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

    inline bool Argument::isOptional() const {
        return !isRequired();
    }

    inline void Argument::setOptional(bool optional) {
        setRequired(!optional);
    }

    class SYSCMDLINE_EXPORT ArgumentHolder : public Symbol {
        SYSCMDLINE_DECL_DATA(ArgumentHolder)
    public:
        ~ArgumentHolder();

    public:
        Argument argument(const std::string &name) const;
        Argument argument(int index) const;
        const std::vector<Argument> &arguments() const;
        int indexOfArgument(const std::string &name) const;
        bool hasArgument(const std::string &name) const;
        void addArgument(const Argument &argument);
        void setArguments(const std::vector<Argument> &arguments);

        std::string displayedArguments(int displayOptions) const;

        using Symbol::helpText;

    protected:
        ArgumentHolder(ArgumentHolderData *d);

        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

}

#endif // ARGUMENT_H
