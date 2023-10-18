#include "argument.h"
#include "argument_p.h"

#include <stdexcept>
#include <sstream>

namespace SysCmdLine {

    ArgumentData::ArgumentData(const std::string &name, const std::string &desc,
                               const std::vector<std::string> &expectedValues,
                               const std::string &defaultValue, bool required)
        : SymbolData(Symbol::ST_Argument, name, desc), expectedValues(expectedValues),
          defaultValue(defaultValue), required(required) {
    }

    ArgumentData::~ArgumentData() {
    }

    SymbolData *ArgumentData::clone() const {
        return new ArgumentData(name, desc, expectedValues, defaultValue, required);
    }

    Argument::Argument() : Argument({}, {}) {
    }

    Argument::Argument(const std::string &name, const std::string &desc, bool required)
        : Argument(name, desc, {}, required) {
    }

    Argument::Argument(const std::string &name, const std::string &desc,
                       const std::string &defaultValue, bool required)
        : Argument(name, desc, {}, defaultValue, required) {
    }

    Argument::Argument(const std::string &name, const std::string &desc,
                       const std::vector<std::string> &expectedValues,
                       const std::string &defaultValue, bool required)
        : Symbol(new ArgumentData(name, desc, expectedValues, defaultValue, required)) {
    }

    Argument::~Argument() {
    }

    Argument::Argument(const Argument &other) : Symbol(other.d_ptr->clone()) {
    }

    Argument::Argument(Argument &&other) noexcept : Argument() {
        d_ptr.swap(other.d_ptr);
    }

    Argument &Argument::operator=(const Argument &other) {
        if (this == &other) {
            return *this;
        }
        d_ptr = other.d_ptr->clone();
        return *this;
    }

    Argument &Argument::operator=(Argument &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        d_ptr.swap(other.d_ptr);
        return *this;
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
        if (argumentNameIndexes.count(arg.name())) {
            throw std::runtime_error("argument name \"" + arg.name() + "\" duplicated");
        }
        if (!arguments.empty() && !arguments.back().isRequired() && arg.isRequired()) {
            throw std::runtime_error(
                "adding required argument after optional arguments is prohibited");
        }
        arguments.push_back(arg);
        argumentNameIndexes.insert(std::make_pair(arg.name(), arguments.size() - 1));
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

    const std::vector<Argument> &ArgumentHolder::arguments() const {
        SYSCMDLINE_GET_CONST_DATA(ArgumentHolder);
        return d->arguments;
    }

    void ArgumentHolder::addArgument(const Argument &argument) {
        SYSCMDLINE_GET_DATA(ArgumentHolder);
        d->addArgument(argument);
    }

    void ArgumentHolder::setArguments(const std::vector<Argument> &arguments) {
        SYSCMDLINE_GET_DATA(ArgumentHolder);
        d->setArguments(arguments);
    }

    std::string ArgumentHolder::displayArgumentList() const {
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
                ss << "<" << _arguments[i].name() << "> ";
            }
            ss << "<" << _arguments[optionalIdx - 1].name() << ">";
        }

        if (optionalIdx < _arguments.size()) {
            ss << " [";
            for (std::string::size_type i = optionalIdx; i < _arguments.size() - 1; ++i) {
                ss << "<" << _arguments[i].name() << "> ";
            }
            ss << "<" << _arguments[_arguments.size()].name() << ">]";
        }

        return ss.str();
    }

    ArgumentHolder::ArgumentHolder(ArgumentHolderData *d) : Symbol(d) {
    }

}