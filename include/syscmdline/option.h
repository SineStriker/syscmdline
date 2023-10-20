#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <vector>

#include <syscmdline/argument.h>

namespace SysCmdLine {

    class OptionData;

    class SYSCMDLINE_EXPORT Option : public ArgumentHolder {
    public:
        enum PriorLevel {
            NoPrior,
            IgnoreMissingArgument,
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

        std::string displayedTokens() const;
        std::string displayedText(bool allTokens = true) const;

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
        OptionData *d_func();
        const OptionData *d_func() const;

        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserPrivate;
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
