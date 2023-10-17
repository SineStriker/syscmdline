#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <string>
#include <vector>

#include <syscmdline/symbol.h>

namespace SysCmdLine {

    template <class T>
    class BasicArgument : public BasicSymbol<T> {
    public:
        using super = BasicSymbol<T>;

        using value_type = typename super::value_type;
        using string_type = typename super::string_type;

        ~BasicArgument() = default;

        BasicArgument() : super(ST_Argument), _required(false) {
        }

        BasicArgument(const string_type &name, bool required = false, const string_type &desc = {})
            : super(ST_Argument, name, desc), _required(required) {
        }

        BasicArgument(const string_type &name, const string_type &defaultValue,
                      bool required = false, const string_type &desc = {})
            : BasicArgument(name, required, desc), _defaultValue(defaultValue) {
        }

        BasicArgument(const string_type &name, const string_type &defaultValue,
                      const std::vector<string_type> &expectedValues, bool required = false,
                      const string_type &desc = {})
            : BasicArgument(name, required, desc), _defaultValue(defaultValue),
              _expectedValues(expectedValues) {
        }

    public:
        std::vector<string_type> expectedValues() const {
            return _expectedValues;
        }

        void setExpectedValues(const std::vector<string_type> &expectedValues) {
            _expectedValues = expectedValues;
        }

        string_type defaultValue() const {
            return _defaultValue;
        }

        void setDefaultValue(const string_type &defaultValue) {
            _defaultValue = defaultValue;
        }

        bool isRequired() const {
            return _required;
        }

        void setRequired(bool required) {
            _required = required;
        }

    private:
        std::vector<string_type> _expectedValues;
        string_type _defaultValue;
        bool _required;
    };

    using Argument = BasicArgument<char>;
    using WArgument = BasicArgument<wchar_t>;

}

#endif // ARGUMENT_H
