#include "argument.h"
#include "argument_p.h"

#include <stdexcept>

#include "strings.h"
#include "parser.h"
#include "utils.h"

namespace SysCmdLine {

    ArgumentData::ArgumentData(const std::string &name, const std::string &desc,
                               const std::vector<Value> &expectedValues, const Value &defaultValue,
                               bool required, const std::string &displayName, bool multipleEnabled)
        : SymbolData(Symbol::ST_Argument, name, desc), defaultValue(defaultValue),
          required(required), displayName(displayName), multiple(multipleEnabled) {
        if (!expectedValues.empty())
            setExpectedValues(expectedValues);
    }

    ArgumentData::~ArgumentData() {
    }

    SymbolData *ArgumentData::clone() const {
        return new ArgumentData(*this);
    }

    void ArgumentData::setExpectedValues(const std::vector<Value> &values) {
        for (const auto &val : values) {
            if (val.toString().empty()) {
                throw std::runtime_error("candidate value cannot be null");
            }
        }
        expectedValues = values;
    }

    Argument::Argument() : Argument(std::string()) {
    }

    Argument::Argument(const std::string &name, const std::string &desc)
        : Argument(name, desc, Value()) {
    }

    Argument::Argument(const std::string &name, const std::string &desc, const Value &defaultValue,
                       bool required, const std::string &displayName, bool multipleEnabled)
        : Argument(name, desc, {}, defaultValue, required, displayName, multipleEnabled) {
    }

    Argument::Argument(const std::string &name, const std::string &desc,
                       const std::vector<Value> &expectedValues, const Value &defaultValue,
                       bool required, const std::string &displayName, bool multipleEnabled)
        : Symbol(new ArgumentData(name, desc, expectedValues, defaultValue, required, displayName,
                                  multipleEnabled)) {
    }

    Argument::~Argument() {
    }

    Argument::Argument(const Argument &other) : Symbol(nullptr) {
        d_ptr = other.d_ptr;
    }

    Argument::Argument(Argument &&other) noexcept : Symbol(nullptr) {
        d_ptr.swap(other.d_ptr);
    }

    Argument &Argument::operator=(const Argument &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr;
        return *this;
    }

    Argument &Argument::operator=(Argument &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
    }

    std::string Argument::displayedText() const {
        SYSCMDLINE_GET_DATA(const Argument);
        std::string res = d->displayName.empty() ? ("<" + d->name + ">") : d->displayName;
        if (d->multiple)
            res += "...";
        return res;
    }

    std::string Argument::helpText(Symbol::HelpPosition pos, int displayOptions,
                                   void *extra) const {
        SYSCMDLINE_GET_DATA(const Argument);
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
                                ": " + Utils::join<char>(values, ", ") + "]";
                }
                return d->desc + appendix;
            }

            default:
                break;
        }
        return displayedText();
    }

    const std::vector<Value> &Argument::expectedValues() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->expectedValues;
    }

    void Argument::setExpectedValues(const std::vector<Value> &expectedValues) {
        if (expectedValues == this->expectedValues())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->expectedValues = expectedValues;
    }

    Value Argument::defaultValue() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->defaultValue;
    }

    void Argument::setDefaultValue(const Value &defaultValue) {
        if (defaultValue == this->defaultValue())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->defaultValue = defaultValue;
    }

    std::string Argument::displayName() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->displayName;
    }

    void Argument::setDisplayName(const std::string &displayName) {
        if (displayName == this->displayName())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->displayName = displayName;
    }

    bool Argument::isRequired() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->required;
    }

    void Argument::setRequired(bool required) {
        if (required == isRequired())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->required = required;
    }

    bool Argument::multiValueEnabled() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->multiple;
    }

    void Argument::setMultiValueEnabled(bool on) {
        if (on == multiValueEnabled())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->multiple = on;
    }

    Argument::Validator Argument::validator() const {
        SYSCMDLINE_GET_DATA(const Argument);
        return d->validator;
    }

    void Argument::setValidator(const Validator &validator) {
        SYSCMDLINE_GET_DATA(Argument);
        d->validator = validator;
    }

    ArgumentHolderData::ArgumentHolderData(Symbol::SymbolType type, const std::string &name,
                                           const std::string &desc,
                                           const std::vector<Argument> &args)
        : SymbolData(type, name, desc), multiValueIndex(-1) {
        if (!args.empty())
            setArguments(args);
    }

    ArgumentHolderData::~ArgumentHolderData() {
    }

    void ArgumentHolderData::addArgument(const Argument &arg) {
        const auto &d = arg.d_func();
        const auto &name = d->name;
        if (name.empty()) {
            throw std::runtime_error("empty argument name");
        }
        if (argumentNameIndexes.count(name)) {
            throw std::runtime_error("argument name \"" + name + "\" duplicated");
        }
        if (!arguments.empty() && !arguments.back().isRequired() && d->required) {
            throw std::runtime_error(
                "adding required argument after optional arguments is prohibited");
        }

        if (arg.multiValueEnabled()) {
            if (type < Symbol::ST_Command) {
                throw std::runtime_error(
                    "adding multi-value argument to non-command symbol is prohibited");
            }
            if (multiValueIndex >= 0) {
                throw std::runtime_error("there can be at most one multi-value argument");
            }
            if (d->defaultValue.type() != Value::Null) {
                throw std::runtime_error("multi-value argument with default value is prohibited");
            }
            multiValueIndex = int(arguments.size());
        } else if (multiValueIndex >= 0 && !d->required) {
            throw std::runtime_error(
                "adding optional argument after multi-value argument is prohibited");
        }

        // check if default value is valid
        {
            const auto &expectedValues = d->expectedValues;
            const auto &defaultValue = d->defaultValue;
            if (!expectedValues.empty() && defaultValue.type() != Value::Null &&
                std::find(expectedValues.begin(), expectedValues.end(), defaultValue) ==
                    expectedValues.end()) {
                throw std::runtime_error("default value \"" + defaultValue.toString() +
                                         "\" is not in expected values");
            }
        }

        argumentNameIndexes.insert(std::make_pair(name, arguments.size()));
        arguments.push_back(arg);
    }

    void ArgumentHolderData::setArguments(const std::vector<Argument> &args) {
        arguments.clear();
        argumentNameIndexes.clear();
        multiValueIndex = -1;
        if (args.empty())
            return;

        arguments.reserve(args.size());
        argumentNameIndexes.reserve(args.size());
        for (const auto &arg : args) {
            addArgument(arg);
        }
    }

    ArgumentHolder::~ArgumentHolder() {
    }

    Argument ArgumentHolder::argument(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        auto it = d->argumentNameIndexes.find(name);
        if (it == d->argumentNameIndexes.end())
            return {};
        return d->arguments[it->second];
    }

    Argument ArgumentHolder::argument(int index) const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        if (index < 0 || index >= d->arguments.size())
            return {};
        return d->arguments[index];
    }

    const std::vector<Argument> &ArgumentHolder::arguments() const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        return d->arguments;
    }

    int ArgumentHolder::indexOfArgument(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        auto it = d->argumentNameIndexes.find(name);
        if (it == d->argumentNameIndexes.end())
            return -1;
        return int(it->second);
    }

    bool ArgumentHolder::hasArgument(const std::string &name) const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        return d->argumentNameIndexes.count(name);
    }

    void ArgumentHolder::addArgument(const Argument &argument) {
        SYSCMDLINE_GET_DATA(ArgumentHolder);
        d->addArgument(argument);
    }

    void ArgumentHolder::setArguments(const std::vector<Argument> &arguments) {
        SYSCMDLINE_GET_DATA(ArgumentHolder);
        d->setArguments(arguments);
    }

    std::string ArgumentHolder::displayedArguments(int displayOptions) const {
        SYSCMDLINE_GET_DATA(const ArgumentHolder);
        const auto &arguments = d->arguments;

        std::string ss;
        std::string::size_type optionalIdx = arguments.size();
        for (std::string::size_type i = 0; i < arguments.size(); ++i) {
            if (!arguments.at(i).isRequired()) {
                optionalIdx = i;
                break;
            }
        }

        if (optionalIdx > 0) {
            for (std::string::size_type i = 0; i < optionalIdx - 1; ++i) {
                ss += arguments[i].helpText(Symbol::HP_Usage, displayOptions, nullptr);
                ss += " ";
            }
            ss += arguments[optionalIdx - 1].helpText(Symbol::HP_Usage, displayOptions, nullptr);
        }

        if (optionalIdx < arguments.size()) {
            ss += " [";
            for (std::string::size_type i = optionalIdx; i < arguments.size() - 1; ++i) {
                ss += arguments[i].helpText(Symbol::HP_Usage, displayOptions, nullptr);
                ss += " ";
            }
            ss +=
                arguments[arguments.size() - 1].helpText(Symbol::HP_Usage, displayOptions, nullptr);
            ss += "]";
        }
        return ss;
    }

    ArgumentHolder::ArgumentHolder(ArgumentHolderData *d) : Symbol(d) {
    }

}