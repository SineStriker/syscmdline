#include "option.h"

#include <stdexcept>
#include <sstream>

#include "strings.h"

namespace SysCmdLine {

    Option::Option() : Symbol(ST_Option), _short(false), _global(false) {
    }

    Option::~Option() {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, const std::vector<Argument> &arguments)
        : Option(name, desc, tokens, false, false, arguments) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, bool is_short, bool global,
                   const std::vector<Argument> &arguments)
        : Symbol(ST_Option, name, desc), ArgumentHolder(arguments), _short(is_short),
          _global(global) {
        if (!tokens.empty())
            setTokens(tokens);
    }

    void Option::setTokens(const std::vector<std::string> &tokens) {
        _tokens.clear();
        if (tokens.empty())
            return;

        _tokens.reserve(tokens.size());
        for (const auto &token : tokens) {
            if (token.empty()) {
                throw std::runtime_error("empty token");
            }
            if (token.front() != '-') {
                throw std::runtime_error("token must start with \"-\"");
            }
            _tokens.push_back(token);
        }
    }

    std::string Option::displayTokens() const {
        std::stringstream ss;
        ss << Strings::join<char>(_tokens, ", ");
        if (!_arguments.empty()) {
            ss << " " << Argument::displayArgumentList(_arguments);
        }
        return ss.str();
    }

}