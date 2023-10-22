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

#ifndef OPTION_H
#define OPTION_H

#include <syscmdline/argument.h>

namespace SysCmdLine {

    class OptionData;

    class SYSCMDLINE_EXPORT Option : public ArgumentHolder {
        SYSCMDLINE_DECL_DATA(Option)
    public:
        enum PriorLevel {
            NoPrior,
            IgnoreMissingSymbols,
            ExclusiveToArguments,
            ExclusiveToOptions,
            ExclusiveToAll,
        };

        enum ShortMatchRule {
            NoShortMatch,
            ShortMatchAll,
            ShortMatchSingleChar,
            ShortMatchSingleLetter,
        };

        Option();
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens = {}, bool required = false,
               const std::vector<Argument> &arguments = {});
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens, bool required, ShortMatchRule shortMatchType,
               PriorLevel priorLevel, bool global, const std::vector<Argument> &arguments = {});
        ~Option();

        Option(const Option &other);
        Option(Option &&other) noexcept;
        Option &operator=(const Option &other);
        Option &operator=(Option &&other) noexcept;

        using Symbol::helpText;
        std::string helpText(HelpPosition pos, int displayOptions, void *extra) const override;

    public:
        const std::vector<std::string> &tokens() const;
        void setTokens(const std::vector<std::string> &tokens);
        inline void setToken(const std::string &token);

        bool isRequired() const;
        void setRequired(bool required);

        inline bool isOptional() const;
        inline void setOptional(bool optional);

        ShortMatchRule shortMatchRule() const;
        void setShortMatchRule(ShortMatchRule shortMatchRule);

        PriorLevel priorLevel() const;
        void setPriorLevel(PriorLevel priorLevel);

        bool isGlobal() const;
        void setGlobal(bool on);

        int maxOccurrence() const;
        void setMaxOccurrence(int max);

    protected:
        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserData;
        friend class ParseResult;
        friend class ParseResultData;
    };

    inline void Option::setToken(const std::string &token) {
        setTokens({token});
    }

    inline bool Option::isOptional() const {
        return !isRequired();
    }

    inline void Option::setOptional(bool optional) {
        setRequired(!optional);
    }

}

#endif // OPTION_H
