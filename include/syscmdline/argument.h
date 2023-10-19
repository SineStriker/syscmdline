#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include <syscmdline/symbol.h>
#include <syscmdline/value.h>

namespace SysCmdLine {

    class Parser;

    class ParserPrivate;

    class ArgumentHolder;

    class ArgumentHolderData;

    class Command;

    class CommandData;

    class ArgumentData;

    class SYSCMDLINE_EXPORT Argument : public Symbol {
    public:
        using Validator = std::function<bool /* result */ (
            const std::string & /* token */, Value * /* out */, std::string * /* errorMessage */)>;

        Argument();
        Argument(const std::string &name, const std::string &desc = {});
        Argument(const std::string &name, const std::string &desc, const Value &defaultValue,
                 bool required = true, const std::string &displayName = {});
        Argument(const std::string &name, const std::string &desc,
                 const std::vector<Value> &expectedValues, const Value &defaultValue,
                 bool required = true, const std::string &displayName = {});
        ~Argument();

        Argument(const Argument &other);
        Argument(Argument &&other) noexcept;
        Argument &operator=(const Argument &other);
        Argument &operator=(Argument &&other) noexcept;

        std::string displayedText() const;

    public:
        const std::vector<Value> &expectedValues() const;
        void setExpectedValues(const std::vector<Value> &expectedValues);

        Value defaultValue() const;
        void setDefaultValue(const Value &defaultValue);

        std::string displayName() const;
        void setDisplayName(const std::string &displayName);

        bool isRequired() const;
        void setRequired(bool required);

        Validator validator() const;
        void setValidator(const Validator &validator);

    protected:
        ArgumentData *d_func();
        const ArgumentData *d_func() const;

        friend class ArgumentHolder;
        friend class ArgumentHolderData;
        friend class Command;
        friend class CommandData;
        friend class Parser;
        friend class ParserPrivate;
    };

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

        ArgumentHolderData *d_func();
        const ArgumentHolderData *d_func() const;

        friend class Parser;
        friend class ParserPrivate;
    };

}

#endif // ARGUMENT_H
