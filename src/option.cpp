#include "option.h"
#include "option_p.h"

#include "utils.h"
#include "strings.h"
#include "parser.h"

namespace SysCmdLine {

    OptionPrivate::OptionPrivate(Option::SpecialType specialType,
                                 const std::vector<std::string> &tokens, const std::string &desc,
                                 bool required)
        : ArgumentHolderPrivate(Symbol::ST_Option, desc), specialType(specialType), tokens(tokens),
          required(required), shortMatchRule(Option::NoShortMatch), priorLevel(Option::NoPrior),
          global(false), maxOccurrence(1) {
    }

    SymbolPrivate *OptionPrivate::clone() const {
        return new OptionPrivate(*this);
    }

    Option::Option(SpecialType specialType)
        : ArgumentHolder(new OptionPrivate(specialType, {}, {}, false)) {
    }

    Option::Option(const std::string &token, const std::string &desc, bool required)
        : Option(std::vector<std::string>{token}, desc, required) {
    }

    Option::Option(const std::vector<std::string> &tokens, const std::string &desc, bool required)
        : ArgumentHolder(new OptionPrivate(NoSpecial, tokens, desc, required)) {
    }

    std::string Option::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        Q_D2(Option);
        if (auto ss = ArgumentHolder::helpText(pos, displayOptions, extra); !ss.empty()) {
            return ss;
        }

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
                auto textProvider = reinterpret_cast<Parser::TextProvider>(extra);
                if (!textProvider) {
                    textProvider = Parser::defaultTextProvider();
                }

                std::string appendix;

                // Required
                if (d->required && (displayOptions & Parser::ShowOptionIsRequired)) {
                    appendix += " [" + textProvider(Strings::Title, Strings::Required) + "]";
                }
                return d->desc + appendix;
            }
        }

        return {};
    }

    const std::vector<std::string> &Option::tokens() const {
        Q_D2(Option);
        return d->tokens;
    }

    void Option::setTokens(const std::vector<std::string> &tokens) {
        Q_D(Option);
        d->tokens = tokens;
    }

    bool Option::isRequired() const {
        Q_D2(Option);
        return d->required;
    }

    void Option::setRequired(bool required) {
        Q_D(Option);
        d->required = required;
    }

    Option::ShortMatchRule Option::shortMatchRule() const {
        Q_D2(Option);
        return d->shortMatchRule;
    }

    void Option::setShortMatchRule(ShortMatchRule shortMatchRule) {
        Q_D(Option);
        d->shortMatchRule = shortMatchRule;
    }

    Option::PriorLevel Option::priorLevel() const {
        Q_D2(Option);
        return d->priorLevel;
    }

    void Option::setPriorLevel(PriorLevel priorLevel) {
        Q_D(Option);
        d->priorLevel = priorLevel;
    }

    bool Option::isGlobal() const {
        Q_D2(Option);
        return d->global;
    }

    void Option::setGlobal(bool on) {
        Q_D(Option);
        d->global = on;
    }

    int Option::maxOccurrence() const {
        Q_D2(Option);
        return d->maxOccurrence;
    }

    void Option::setMaxOccurrence(int max) {
        Q_D(Option);
        d->global = max;
    }

    Option::SpecialType Option::specialType() const {
        Q_D2(Option);
        return d->specialType;
    }

    void Option::setSpecialType(Option::SpecialType specialType) {
        Q_D(Option);
        d->specialType = specialType;
    }

}