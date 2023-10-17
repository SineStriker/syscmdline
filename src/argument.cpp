#include "argument.h"

namespace SysCmdLine {

    Argument::Argument() : Symbol(ST_Argument), _required(false) {
    }

    Argument::Argument(const std::string &name, bool required, const std::string &desc)
        : Argument(name, {}, required, desc) {
    }

    Argument::Argument(const std::string &name, const std::string &defaultValue, bool required,
                       const std::string &desc)
        : Argument(name, defaultValue, {}, required, desc) {
    }

    Argument::Argument(const std::string &name, const std::string &defaultValue,
                       const std::vector<std::string> &expectedValues, bool required,
                       const std::string &desc)
        : Symbol(ST_Argument, name, desc), _defaultValue(defaultValue), _required(required),
          _expectedValues(expectedValues) {
    }

    Argument::~Argument() {
    }

    ArgumentHolder::ArgumentHolder(const std::vector<Argument> &arguments) {
        setArguments(arguments);
    }

    ArgumentHolder::~ArgumentHolder() {
    }

    void ArgumentHolder::addArgument(const Argument &argument) {
        if (_argumentNameIndexes.count(argument.name())) {
            throw std::runtime_error("argument name \"" + argument.name() + "\" duplicated");
        }
        if (!_arguments.empty() && !_arguments.back().isRequired() && argument.isRequired()) {
            throw std::runtime_error(
                "adding required argument after optional arguments is prohibited");
        }
        _arguments.push_back(argument);
        _argumentNameIndexes.insert(std::make_pair(argument.name(), _arguments.size() - 1));
    }

    void ArgumentHolder::setArguments(const std::vector<Argument> &arguments) {
        _arguments.clear();
        _argumentNameIndexes.clear();
        if (arguments.empty())
            return;

        _arguments.reserve(arguments.size());
        _argumentNameIndexes.reserve(arguments.size());
        for (const auto &arg : arguments) {
            addArgument(arg);
        }
    }

}