#include "option.h"

namespace SysCmdLine {

    Option::Option() : Symbol(ST_Option), _short(false) {
    }

    Option::~Option() {
    }

    Option::Option(const std::string &name, const std::vector<std::string> &tokens,
                   const std::vector<Argument> &arguments, const std::string &desc)
        : Option(name, tokens, false, false, arguments, desc) {
    }

    Option::Option(const std::string &name, const std::vector<std::string> &tokens, bool is_short,
                   bool prior, const std::vector<Argument> &arguments, const std::string &desc)
        : Symbol(ST_Option, name, desc), ArgumentHolder(arguments), _tokens(tokens),
          _short(is_short), _prior(prior) {
    }
    
}