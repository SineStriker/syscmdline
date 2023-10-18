#include "option.h"
#include "option_p.h"

#include <stdexcept>
#include <sstream>

#include "strings.h"

namespace SysCmdLine {

    OptionData::OptionData(const std::string &name, const std::string &desc,
                           const std::vector<std::string> &tokens, bool required,
                           const std::vector<Argument> &args, bool is_short,
                           Option::PriorLevel priorLevel, bool global, int maxOccurrence)
        : ArgumentHolderData(Symbol::ST_Option, name, desc, args), tokens(tokens),
          required(required), is_short(is_short), priorLevel(priorLevel), global(global),
          maxOccurrence(maxOccurrence) {

        if (!tokens.empty())
            setTokens(tokens);
    }

    OptionData::~OptionData() {
    }

    SymbolData *OptionData::clone() const {
        return new OptionData(name, desc, tokens, required, arguments, is_short, priorLevel, global,
                              maxOccurrence);
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
                   const std::vector<std::string> &tokens, bool required,
                   const std::vector<Argument> &arguments)
        : Option(name, desc, tokens, required, false, NoPrior, false, arguments) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, bool required, bool is_short,
                   Option::PriorLevel priorLevel, bool global,
                   const std::vector<Argument> &arguments)
        : ArgumentHolder(new OptionData(name, desc, tokens, required, arguments, is_short,
                                        priorLevel, global, 0)) {
    }

    Option::~Option() {
    }

    Option::Option(const Option &other) : ArgumentHolder(nullptr) {
        d_ptr = other.d_ptr;
    }

    Option::Option(Option &&other) noexcept : ArgumentHolder(nullptr) {
        d_ptr.swap(other.d_ptr);
    }

    Option &Option::operator=(const Option &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    Option &Option::operator=(Option &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    std::string Option::displayedTokens() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return Strings::join<char>(d->tokens, ", ");
    }

    std::string Option::displayedText(bool allTokens) const {
        SYSCMDLINE_GET_CONST_DATA(Option);

        const auto &_arguments = d->arguments;
        const auto &_tokens = d->tokens;

        std::stringstream ss;
        ss << (allTokens ? Strings::join<char>(_tokens, ", ") : _tokens.front());
        if (!_arguments.empty()) {
            ss << " " << displayedArguments();
        }
        return ss.str();
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

    bool Option::isRequired() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->required;
    }

    void Option::setRequired(bool required) {
        if (required == this->isRequired())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->required = required;
    }

    bool Option::isShortOption() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->is_short;
    }

    void Option::setShortOption(bool on) {
        if (on == this->isShortOption())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->is_short = on;
    }

    Option::PriorLevel Option::priorLevel() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->priorLevel;
    }

    void Option::setPriorLevel(PriorLevel priorLevel) {
        if (priorLevel == this->priorLevel())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->priorLevel = priorLevel;
    }

    bool Option::isGlobal() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->global;
    }

    void Option::setGlobal(bool on) {
        if (on == this->isGlobal())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->global = on;
    }

    int Option::maxOccurrence() const {
        SYSCMDLINE_GET_CONST_DATA(Option);
        return d->maxOccurrence;
    }

    void Option::setMaxOccurrence(int max) {
        if (max == this->maxOccurrence())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->global = max;
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