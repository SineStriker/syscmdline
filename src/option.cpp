#include "option.h"
#include "option_p.h"

#include <stdexcept>

#include "utils.h"
#include "strings.h"
#include "parser.h"

namespace SysCmdLine {

    OptionData::OptionData(const std::string &name, const std::string &desc,
                           const std::vector<std::string> &tokens, bool required,
                           const std::vector<Argument> &args, Option::ShortMatchRule shortMatchRule,
                           Option::PriorLevel priorLevel, bool global, int maxOccurrence)
        : ArgumentHolderData(Symbol::ST_Option, name, desc, args), tokens(tokens),
          required(required), shortMatchRule(shortMatchRule), priorLevel(priorLevel),
          global(global), maxOccurrence(maxOccurrence) {

        if (!tokens.empty())
            setTokens(tokens);
    }

    OptionData::~OptionData() {
    }

    SymbolData *OptionData::clone() const {
        return new OptionData(*this);
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
            if (token.front() != '-' && token.front() != '/') {
                throw std::runtime_error(R"(token must start with "-" or "/")");
            }
            _tokens.push_back(token);
        }
    }

    Option::Option() : Option({}, {}, {}) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, bool required,
                   const std::vector<Argument> &arguments)
        : Option(name, desc, tokens, required, NoShortMatch, NoPrior, false, arguments) {
    }

    Option::Option(const std::string &name, const std::string &desc,
                   const std::vector<std::string> &tokens, bool required,
                   ShortMatchRule shortMatchType, Option::PriorLevel priorLevel, bool global,
                   const std::vector<Argument> &arguments)
        : ArgumentHolder(new OptionData(name, desc, tokens, required, arguments, shortMatchType,
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

    std::string Option::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        SYSCMDLINE_GET_DATA(const Option);
        if (d->helpProvider)
            return d->helpProvider(this, pos, displayOptions, extra);

        switch (pos) {
            case Symbol::HP_Usage: {
                std::string appendix;
                if (!d->arguments.empty()) {
                    appendix = " " + displayedArguments(displayOptions);
                }
                return d->tokens.front() + appendix;
            }
            case Symbol::HP_ErrorText: {
                return d->tokens.front();
            }
            case Symbol::HP_FirstColumn: {
                std::string appendix;
                if (!d->arguments.empty()) {
                    appendix = " " + displayedArguments(displayOptions);
                }
                return Utils::join(d->tokens, ", ") + appendix;
            }
            case Symbol::HP_SecondColumn: {
                std::string appendix;

                // Required
                if (d->required && (displayOptions & Parser::ShowOptionIsRequired)) {
                    appendix += " [" + Strings::text(Strings::Title, Strings::Required) + "]";
                }
                return d->desc + appendix;
            }
        }

        return {};
    }

    const std::vector<std::string> &Option::tokens() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->tokens;
    }

    void Option::setTokens(const std::vector<std::string> &tokens) {
        if (tokens == this->tokens())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->setTokens(tokens);
    }

    bool Option::isRequired() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->required;
    }

    void Option::setRequired(bool required) {
        if (required == this->isRequired())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->required = required;
    }

    Option::ShortMatchRule Option::shortMatchRule() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->shortMatchRule;
    }

    void Option::setShortMatchRule(ShortMatchRule shortMatchRule) {
        if (shortMatchRule == this->shortMatchRule())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->shortMatchRule = shortMatchRule;
    }

    Option::PriorLevel Option::priorLevel() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->priorLevel;
    }

    void Option::setPriorLevel(PriorLevel priorLevel) {
        if (priorLevel == this->priorLevel())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->priorLevel = priorLevel;
    }

    bool Option::isGlobal() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->global;
    }

    void Option::setGlobal(bool on) {
        if (on == this->isGlobal())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->global = on;
    }

    int Option::maxOccurrence() const {
        SYSCMDLINE_GET_DATA(const Option);
        return d->maxOccurrence;
    }

    void Option::setMaxOccurrence(int max) {
        if (max == this->maxOccurrence())
            return;

        SYSCMDLINE_GET_DATA(Option);
        d->global = max;
    }

}