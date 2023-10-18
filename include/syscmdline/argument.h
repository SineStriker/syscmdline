#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <string>
#include <vector>
#include <unordered_map>

#include <syscmdline/symbol.h>

namespace SysCmdLine {

    class Parser;
    class ParserPrivate;

    class Command;

    class SYSCMDLINE_EXPORT Argument : public Symbol {
    public:
        Argument();
        Argument(const std::string &name, const std::string &desc, bool required = true);
        Argument(const std::string &name, const std::string &desc, const std::string &defaultValue,
                 bool required = true);
        Argument(const std::string &name, const std::string &desc,
                 const std::vector<std::string> &expectedValues, const std::string &defaultValue,
                 bool required = true);
        ~Argument();

    public:
        inline const std::vector<std::string> &expectedValues() const;
        inline void setExpectedValues(const std::vector<std::string> &expectedValues);

        inline std::string defaultValue() const;
        inline void setDefaultValue(const std::string &defaultValue);

        inline bool isRequired() const;
        inline void setRequired(bool required);

    protected:
        std::vector<std::string> _expectedValues;
        std::string _defaultValue;
        bool _required;

        friend class Command;
        friend class Parser;
        friend class ParserPrivate;
    };

    inline const std::vector<std::string> &Argument::expectedValues() const {
        return _expectedValues;
    }

    inline void Argument::setExpectedValues(const std::vector<std::string> &expectedValues) {
        _expectedValues = expectedValues;
    }

    inline std::string Argument::defaultValue() const {
        return _defaultValue;
    }

    inline void Argument::setDefaultValue(const std::string &defaultValue) {
        _defaultValue = defaultValue;
    }

    inline bool Argument::isRequired() const {
        return _required;
    }

    inline void Argument::setRequired(bool required) {
        _required = required;
    }

    class SYSCMDLINE_EXPORT ArgumentHolder {
    public:
        ArgumentHolder(const std::vector<Argument> &arguments = {});
        ~ArgumentHolder();

    public:
        inline const std::vector<Argument> &arguments() const;
        inline const Argument &argument(const std::string &name) const;
        inline const Argument &argument(int index) const;

        void addArgument(const Argument &argument);
        void setArguments(const std::vector<Argument> &arguments);

        std::string displayArgumentList() const;

    protected:
        std::vector<Argument> _arguments;
        std::unordered_map<std::string, size_t> _argumentNameIndexes;

        friend class Parser;
        friend class ParserPrivate;
    };

    inline const std::vector<Argument> &ArgumentHolder::arguments() const {
        return _arguments;
    }

    inline const Argument &ArgumentHolder::argument(const std::string &name) const {
        return _arguments.at(_argumentNameIndexes.find(name)->second);
    }

    inline const Argument &ArgumentHolder::argument(int index) const {
        return _arguments.at(index);
    }

}

#endif // ARGUMENT_H
