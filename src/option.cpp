#include "option.h"
#include "option_p.h"

#include <stdexcept>
#include <sstream>

#include "strings.h"

namespace SysCmdLine {

    OptionData::OptionData(const std::string &name, const std::string &desc,
                           const std::vector<std::string> &tokens,
                           const std::vector<Argument> &args, bool is_short, bool prior,
                           bool global)
        : ArgumentHolderData(Symbol::ST_Option, name, desc, args), tokens(tokens),
          is_short(is_short), prior(prior), global(global) {

        if (!tokens.empty())
            setTokens(tokens);
    }

    OptionData::~OptionData() {
    }

    SymbolData *OptionData::clone() const {
        return new OptionData(name, desc, tokens, arguments, is_short, prior, global);
    }

    void OptionData::setTokens(const std::vector<std::string> &tokens) {
        auto &_tokens = this->tokens;

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

    Option::Option() : Option({}, {}, {}) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, const std::vector<Argument> &arguments)
        : Option(name, desc, tokens, false, false, false, arguments) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, bool is_short, bool prior, bool global,
                   const std::vector<Argument> &arguments)
        : ArgumentHolder(new OptionData(name, desc, tokens, arguments, is_short, prior, global)) {
    }

    Option::~Option() {
    }

    Option::Option(const Option &other)
        : ArgumentHolder(static_cast<OptionData *>(other.d_ptr->clone())) {
    }

    Option::Option(Option &&other) noexcept : Option() {
        d_ptr.swap(other.d_ptr);
    }

    Option &Option::operator=(const Option &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr->clone();
        return *this;
    }

    Option &Option::operator=(Option &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    const std::vector<std::string> &Option::tokens() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->tokens;
    }

    void Option::setTokens(const std::vector<std::string> &tokens) {
        if (tokens == this->tokens())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->setTokens(tokens);
    }

    bool Option::isShortOption() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->is_short;
    }

    void Option::setShortOption(bool on) {
        SYSCMDLINE_GET_DATA(Option);
        d->is_short = on;
    }

    bool Option::isPrior() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->prior;
    }

    void Option::setPrior(bool on) {
        SYSCMDLINE_GET_DATA(Option);
        d->prior = on;
    }

    bool Option::isGlobal() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->global;
    }

    void Option::setGlobal(bool on) {
        SYSCMDLINE_GET_DATA(Option);
        d->global = on;
    }

    std::string Option::displayTokens() const {
        SYSCMDLINE_GET_CONST_DATA(Option);

        const auto &_arguments = d->arguments;
        const auto &_tokens = d->tokens;

        std::stringstream ss;
        ss << Strings::join<char>(_tokens, ", ");
        if (!_arguments.empty()) {
            ss << " " << displayArgumentList();
        }
        return ss.str();
    }

    OptionData *Option::d_func() {
        SYSCMDLINE_GET_DATA(Option);
        return d;
    }

    const OptionData *Option::d_func() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d;
    }

}