#include "argument.h"

#include <stdexcept>
#include <sstream>

namespace SysCmdLine {

    Argument::Argument() : Symbol(ST_Argument), _required(true) {
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
        : Symbol(ST_Argument, name, desc), _expectedValues(expectedValues),
          _defaultValue(defaultValue), _required(required) {
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

    std::string ArgumentHolder::displayArgumentList() const {
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

}