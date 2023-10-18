#include "argument.h"
#include "argument_p.h"

#include <stdexcept>
#include <sstream>

namespace SysCmdLine {

    ArgumentData::ArgumentData(const std::string &name, const std::string &desc,
                               const std::vector<std::string> &expectedValues,
                               const std::string &defaultValue, bool required,
                               const std::string &displayName)
        : SymbolData(Symbol::ST_Argument, name, desc), expectedValues(expectedValues),
          defaultValue(defaultValue), required(required), displayName(displayName) {
    }

    ArgumentData::~ArgumentData() {
    }

    SymbolData *ArgumentData::clone() const {
        return new ArgumentData(name, desc, expectedValues, defaultValue, required, displayName);
    }

    Argument::Argument() : Argument({}, {}) {
    }

    Argument::Argument(const std::string &name, const std::string &desc, bool required,
                       const std::string &displayName)
        : Argument(name, desc, {}, required, displayName) {
    }

    Argument::Argument(const std::string &name, const std::string &desc,
                       const std::string &defaultValue, bool required, const std::string &displayName)
        : Argument(name, desc, {}, defaultValue, required, displayName) {
    }

    Argument::Argument(const std::string &name, const std::string &desc,
                       const std::vector<std::string> &expectedValues,
                       const std::string &defaultValue, bool required, const std::string &displayName)
        : Symbol(new ArgumentData(name, desc, expectedValues, defaultValue, required, displayName)) {
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
        SYSCMDLINE_GET_CONST_DATA(Argument);
        if (d->displayName.empty()) {
            return "<" + d->name + ">";
        }
        return d->displayName;
    }

    const std::vector<std::string> &Argument::expectedValues() const {
        SYSCMDLINE_GET_CONST_DATA(Argument);
        return d->expectedValues;
    }

    void Argument::setExpectedValues(const std::vector<std::string> &expectedValues) {
        if (expectedValues == this->expectedValues())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->expectedValues = expectedValues;
    }

    std::string Argument::defaultValue() const {
        SYSCMDLINE_GET_CONST_DATA(Argument);
        return d->defaultValue;
    }

    void Argument::setDefaultValue(const std::string &defaultValue) {
        if (defaultValue == this->defaultValue())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->defaultValue = defaultValue;
    }

    std::string Argument::displayName() const {
        SYSCMDLINE_GET_CONST_DATA(Argument);
        return d->displayName;
    }

    void Argument::setDisplayName(const std::string &displayName) {
        if (displayName == this->displayName())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->displayName = displayName;
    }

    bool Argument::isRequired() const {
        SYSCMDLINE_GET_CONST_DATA(Argument);
        return d->required;
    }

    void Argument::setRequired(bool required) {
        if (required == isRequired())
            return;

        SYSCMDLINE_GET_DATA(Argument);
        d->required = required;
    }

    ArgumentData *Argument::d_func() {
        SYSCMDLINE_GET_DATA(Argument);
        return d;
    }

    const ArgumentData *Argument::d_func() const {
        SYSCMDLINE_GET_CONST_DATA(Argument);
        return d;
    }

    ArgumentHolderData::ArgumentHolderData(Symbol::SymbolType type, const std::string &name,
                                           const std::string &desc,
                                           const std::vector<Argument> &args)
        : SymbolData(type, name, desc) {
        if (!args.empty())
            setArguments(args);
    }

    ArgumentHolderData::~ArgumentHolderData() {
    }

    void ArgumentHolderData::addArgument(const Argument &arg) {
        const auto &name = arg.name();
        if (name.empty()) {
            throw std::runtime_error("empty argument name");
        }
        if (argumentNameIndexes.count(name)) {
            throw std::runtime_error("argument name \"" + name + "\" duplicated");
        }
        if (!arguments.empty() && !arguments.back().isRequired() && arg.isRequired()) {
            throw std::runtime_error(
                "adding required argument after optional arguments is prohibited");
        }
        argumentNameIndexes.insert(std::make_pair(name, arguments.size()));
        arguments.push_back(arg);
    }

    void ArgumentHolderData::setArguments(const std::vector<Argument> &args) {
        arguments.clear();
        argumentNameIndexes.clear();
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

    std::string ArgumentHolder::displayedArguments() const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        auto &_arguments = d->arguments;

        std::stringstream ss;

        std::string::size_type optionalIdx = _arguments.size();
        for (std::string::size_type i = 0; i < _arguments.size(); ++i) {
            if (!_arguments.at(i).isRequired()) {
                optionalIdx = i;
                break;
            }
        }

        if (optionalIdx > 0) {
            for (std::string::size_type i = 0; i < optionalIdx - 1; ++i) {
                ss << _arguments[i].displayedText() << " ";
            }
            ss << _arguments[optionalIdx - 1].displayedText();
        }

        if (optionalIdx < _arguments.size()) {
            ss << " [";
            for (std::string::size_type i = optionalIdx; i < _arguments.size() - 1; ++i) {
                ss << _arguments[i].displayedText() << " ";
            }
            ss << _arguments[_arguments.size() - 1].displayedText() << "]";
        }

        return ss.str();
    }

    Argument ArgumentHolder::argument(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        auto it = d->argumentNameIndexes.find(name);
        if (it == d->argumentNameIndexes.end())
            return {};
        return d->arguments[it->second];
    }

    Argument ArgumentHolder::argument(int index) const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        if (index < 0 || index >= d->arguments.size())
            return {};
        return d->arguments[index];
    }

    const std::vector<Argument> &ArgumentHolder::arguments() const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        return d->arguments;
    }

    int ArgumentHolder::indexOfArgument(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        auto it = d->argumentNameIndexes.find(name);
        if (it == d->argumentNameIndexes.end())
            return -1;
        return it->second;
    }

    bool ArgumentHolder::hasArgument(const std::string &name) const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
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

    ArgumentHolder::ArgumentHolder(ArgumentHolderData *d) : Symbol(d) {
    }

}