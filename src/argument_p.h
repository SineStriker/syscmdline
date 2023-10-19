#ifndef ARGUMENT_P_H
#define ARGUMENT_P_H

#include "argument.h"
#include "symbol_p.h"

namespace SysCmdLine {

    class ArgumentData : public SymbolData {
    public:
        ArgumentData(const std::string &name, const std::string &desc,
                     const std::vector<Value> &expectedValues, const Value &defaultValue,
                     bool required, const std::string &displayName, bool multipleEnabled,
                     const Argument::Validator &validator);
        ~ArgumentData();

        SymbolData *clone() const override;

    public:
        void setExpectedValues(const std::vector<Value> &values);

        std::vector<Value> expectedValues;
        Value defaultValue;
        bool required;
        std::string displayName;
        bool multiple;
        Argument::Validator validator;
    };

    class ArgumentHolderData : public SymbolData {
    public:
        ArgumentHolderData(Symbol::SymbolType type, const std::string &name,
                           const std::string &desc, const std::vector<Argument> &args);
        ~ArgumentHolderData();

    public:
        void addArgument(const Argument &arg);
        void setArguments(const std::vector<Argument> &args);

        int multiValueIndex;
        std::vector<Argument> arguments;
        std::unordered_map<std::string, size_t> argumentNameIndexes;
    };

}

#endif // ARGUMENT_P_H
