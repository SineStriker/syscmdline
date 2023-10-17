#ifndef ARGUMENTHOLDER_H
#define ARGUMENTHOLDER_H

#include <string>
#include <vector>
#include <unordered_map>

#include <syscmdline/argument.h>

namespace SysCmdLine {

    template <class T>
    class BasicArgumentHolder {
    public:
        using value_type = T;
        using string_type = std::basic_string<T>;

        using arg_type = BasicArgument<value_type>;

        BasicArgumentHolder(const std::vector<arg_type> &arguments = {}) {
            setArguments(arguments);
        }

    public:
        std::vector<arg_type> arguments() const {
            return _arguments;
        }

        void addArgument(const arg_type &argument) {
            if (_argumentNameIndexes.count(argument.name())) {
                throw std::runtime_error("argument name duplicated");
            }
            if (!_arguments.empty() && !_arguments.back().isRequired() && argument.isRequired()) {
                throw std::runtime_error(
                    "adding required argument after optional arguments is prohibited");
            }
            _arguments.push_back(argument);
        }

        void setArguments(const std::vector<arg_type> &arguments) {
            _arguments.clear();
            if (arguments.empty())
                return;

            _arguments.reserve(arguments.size());
            for (const auto &arg : arguments) {
                addArgument(arg);
            }
        }

    private:
        std::vector<arg_type> _arguments;
        std::unordered_map<string_type, size_t> _argumentNameIndexes;
    };

}

#endif // ARGUMENTHOLDER_H
