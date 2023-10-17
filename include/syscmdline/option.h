#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <vector>

#include <syscmdline/argument.h>

namespace SysCmdLine {

    class SYSCMDLINE_EXPORT Option : public Symbol, public ArgumentHolder {
    public:
        Option();
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens, const std::vector<Argument> &arguments = {});
        Option(const std::string &name, const std::string &desc,
               const std::vector<std::string> &tokens, bool is_short, bool global,
               const std::vector<Argument> &arguments = {});
        ~Option();

    public:
        inline std::vector<std::string> tokens() const;
        void setTokens(const std::vector<std::string> &tokens);

        inline bool isShortOption() const;
        inline void setShortOption(bool on);

        inline bool isGlobal() const;
        inline void setGlobal(bool on);

        std::string displayTokens() const;

    protected:
        std::vector<std::string> _tokens;
        bool _short;
        bool _global;

        friend class Parser;
        friend class ParserPrivate;
    };

    inline std::vector<std::string> Option::tokens() const {
        return _tokens;
    }

    inline bool Option::isShortOption() const {
        return _short;
    }

    inline void Option::setShortOption(bool on) {
        _short = on;
    }

    inline bool Option::isGlobal() const {
        return _global;
    }

    inline void Option::setGlobal(bool on) {
        _global = on;
    }

}

#endif // OPTION_H
