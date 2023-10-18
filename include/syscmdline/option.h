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
            IgnoreRedundantArgument,
        };

        Option();
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens, const std::vector<Argument> &arguments = {});
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens, bool is_short, PriorLevel priorLevel,
               bool global, const std::vector<Argument> &arguments = {});
        ~Option();

        Option(const Option &other);
        Option(Option &&other) noexcept;
        Option &operator=(const Option &other);
        Option &operator=(Option &&other) noexcept;

        std::string displayedTokens() const;

    public:
        const std::vector<std::string> &tokens() const;
        void setTokens(const std::vector<std::string> &tokens);

        bool isShortOption() const;
        void setShortOption(bool on);

        PriorLevel priorLevel() const;
        void setPriorLevel(PriorLevel priorLevel);

        bool isGlobal() const;
        void setGlobal(bool on);

    protected:
        OptionData *d_func();
        const OptionData *d_func() const;

        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserPrivate;
    };

}

#endif // OPTION_H
