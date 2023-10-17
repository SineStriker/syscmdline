#ifndef OPTION_H
#define OPTION_H

#include <string>
#include <vector>

#include <syscmdline/argument.h>

namespace SysCmdLine {

    class Option : public Symbol, public ArgumentHolder {
    public:
        Option();
        Option(const std::string &name, const std::vector<std::string> &tokens,
               const std::vector<Argument> &arguments = {}, const std::string &desc = {});
        Option(const std::string &name, const std::vector<std::string> &tokens, bool is_short,
               bool prior, const std::vector<Argument> &arguments, const std::string &desc = {});
        ~Option();

    public:
        inline std::vector<std::string> tokens() const;
        inline void setTokens(const std::vector<std::string> &tokens);

        inline bool isShortOption() const;
        inline void setShortOption(bool on);

        inline bool isPrior() const;
        inline void setPrior(bool on);

    protected:
        std::vector<std::string> _tokens;
        bool _short;
        bool _prior;
    };

    inline std::vector<std::string> Option::tokens() const {
        return _tokens;
    }

    inline void Option::setTokens(const std::vector<std::string> &tokens) {
        _tokens = tokens;
    }

    inline bool Option::isShortOption() const {
        return _short;
    }

    inline void Option::setShortOption(bool on) {
        _short = on;
    }

    inline bool Option::isPrior() const {
        return _prior;
    }

    inline void Option::setPrior(bool on) {
        _prior = on;
    }

}

#endif // OPTION_H
