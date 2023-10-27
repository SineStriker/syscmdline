#include "argument.h"
#include "argument_p.h"

#include <stdexcept>

#include "strings.h"
#include "parser.h"
#include "utils.h"

namespace SysCmdLine {

    ArgumentPrivate::ArgumentPrivate(std::string name, const std::string &desc, bool required,
                                     Value defaultValue)
        : SymbolPrivate(Symbol::ST_Argument, desc), name(std::move(name)), required(required),
          defaultValue(std::move(defaultValue)), multiple(false) {
    }

    SymbolPrivate *ArgumentPrivate::clone() const {
        return new ArgumentPrivate(*this);
    }

    Argument::Argument() : Argument(std::string()) {
    }

    Argument::Argument(const std::string &name, const std::string &desc, bool required,
                       const Value &defaultValue)
        : Symbol(new ArgumentPrivate(name, desc, required, defaultValue)) {
    }

    std::string Argument::displayedText() const {
        Q_D2(Argument);
        std::string res = d->displayName.empty() ? ("<" + d->name + ">") : d->displayName;
        if (d->multiple)
            res += "...";
        return res;
    }

    std::string Argument::helpText(Symbol::HelpPosition pos, int displayOptions,
                                   void *extra) const {
        Q_D2(Argument);
        if (d->helpProvider)
            return d->helpProvider(this, pos, displayOptions, extra);

        switch (pos) {
            case Symbol::HP_SecondColumn: {
                std::string appendix;
                // Required
                if (d->required && (displayOptions & Parser::ShowArgumentIsRequired)) {
                    appendix += " [" + Strings::text(Strings::Title, Strings::Required) + "]";
                }

                // Default Value
                if (d->defaultValue.type() != Value::Null &&
                    (displayOptions & Parser::ShowArgumentDefaultValue)) {
                    appendix += " [" + Strings::text(Strings::Title, Strings::Default) + ": " +
                                d->defaultValue.toString() + "]";
                }

                // Expected Values
                if (!d->expectedValues.empty() &&
                    (displayOptions & Parser::ShowArgumentExpectedValues)) {
                    std::vector<std::string> values;
                    values.reserve(d->expectedValues.size());
                    for (const auto &item : d->expectedValues) {
                        switch (item.type()) {
                            case Value::String:
                                values.push_back("\"" + item.toString() + "\"");
                                break;
                            default:
                                values.push_back(item.toString());
                                break;
                        }
                    }
                    appendix += " [" + Strings::text(Strings::Title, Strings::ExpectedValues) +
                                ": " + Utils::join(values, ", ") + "]";
                }
                return d->desc + appendix;
            }

            default:
                break;
        }
        return displayedText();
    }

    std::string Argument::name() const {
        Q_D2(Argument);
        return d->name;
    }

    void Argument::setName(const std::string &name) {
        Q_D(Argument);
        d->name = name;
    }

    const std::vector<Value> &Argument::expectedValues() const {
        Q_D2(Argument);
        return d->expectedValues;
    }

    void Argument::setExpectedValues(const std::vector<Value> &expectedValues) {
        Q_D(Argument);
        d->expectedValues = expectedValues;
    }

    Value Argument::defaultValue() const {
        Q_D2(Argument);
        return d->defaultValue;
    }

    void Argument::setDefaultValue(const Value &defaultValue) {
        Q_D(Argument);
        d->defaultValue = defaultValue;
    }

    std::string Argument::displayName() const {
        Q_D2(Argument);
        return d->displayName;
    }

    void Argument::setDisplayName(const std::string &displayName) {
        Q_D(Argument);
        d->displayName = displayName;
    }

    bool Argument::isRequired() const {
        Q_D2(Argument);
        return d->required;
    }

    void Argument::setRequired(bool required) {
        Q_D(Argument);
        d->required = required;
    }

    bool Argument::multiValueEnabled() const {
        Q_D2(Argument);
        return d->multiple;
    }

    void Argument::setMultiValueEnabled(bool on) {
        Q_D(Argument);
        d->multiple = on;
    }

    Argument::Validator Argument::validator() const {
        Q_D2(Argument);
        return d->validator;
    }

    void Argument::setValidator(const Validator &validator) {
        Q_D(Argument);
        d->validator = validator;
    }

    ArgumentHolderPrivate::ArgumentHolderPrivate(Symbol::SymbolType type, const std::string &desc)
        : SymbolPrivate(type, desc) {
    }

    std::string ArgumentHolder::displayedArguments(int displayOptions) const {
        Q_D2(ArgumentHolder);

        // Use C Style to traverse
        auto arguments = d->arguments.data();
        int size = int(d->arguments.size());

        std::string ss;
        int optionalIdx = size;
        for (int i = 0; i < size; ++i) {
            if (!arguments[i].isRequired()) {
                optionalIdx = i;
                break;
            }
        }

        if (optionalIdx > 0) {
            for (int i = 0; i < optionalIdx - 1; ++i) {
                ss += arguments[i].helpText(Symbol::HP_Usage, displayOptions, nullptr);
                ss += " ";
            }
            ss += arguments[optionalIdx - 1].helpText(Symbol::HP_Usage, displayOptions, nullptr);
        }

        if (optionalIdx < size) {
            ss += " [";
            for (int i = optionalIdx; i < size - 1; ++i) {
                ss += arguments[i].helpText(Symbol::HP_Usage, displayOptions, nullptr);
                ss += " ";
            }
            ss += arguments[size - 1].helpText(Symbol::HP_Usage, displayOptions, nullptr);
            ss += "]";
        }
        return ss;
    }

    int ArgumentHolder::argumentCount() const {
        Q_D2(ArgumentHolder);
        return int(d->arguments.size());
    }

    Argument ArgumentHolder::argument(int index) const {
        Q_D2(ArgumentHolder);
        return d->arguments[index];
    }

    void ArgumentHolder::addArguments(const std::vector<Argument> &arguments) {
        Q_D(ArgumentHolder);
        d->arguments.reserve(d->arguments.size() + arguments.size());
        for (const auto &item : arguments)
            d->arguments.push_back(item);
    }

    ArgumentHolder::ArgumentHolder(ArgumentHolderPrivate *d) : Symbol(d) {
    }

}