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

    class CommandData;

    class ArgumentData;

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

        Argument(const Argument &other);
        Argument(Argument &&other) noexcept;
        Argument &operator=(const Argument &other);
        Argument &operator=(Argument &&other) noexcept;

    public:
        const std::vector<std::string> &expectedValues() const;
        void setExpectedValues(const std::vector<std::string> &expectedValues);

        std::string defaultValue() const;
        void setDefaultValue(const std::string &defaultValue);

        bool isRequired() const;
        void setRequired(bool required);

    protected:
        ArgumentData *d_func();
        const ArgumentData *d_func() const;

        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserPrivate;
    };

    class ArgumentHolderData;

    class SYSCMDLINE_EXPORT ArgumentHolder : public Symbol {
    public:
        ~ArgumentHolder();

        std::string displayedArguments() const;

    public:
        Argument argument(const std::string &name) const;
        Argument argument(int index) const;
        const std::vector<Argument> &arguments() const;
        int indexOfArgument(const std::string &name) const;
        bool hasArgument(const std::string &name) const;
        void addArgument(const Argument &argument);
        void setArguments(const std::vector<Argument> &arguments);

    protected:
        ArgumentHolder(ArgumentHolderData *d);
    };

}

#endif // ARGUMENT_H
