#ifndef ARGUMENT_P_H
#define ARGUMENT_P_H

#include "argument.h"
#include "symbol_p.h"

namespace SysCmdLine {

    class ArgumentData : public SymbolData {
    public:
        ArgumentData(const std::string &name, const std::string &desc,
                     const std::vector<std::string> &expectedValues, const Value &defaultValue,
                     bool required, const std::string &displayName,
                     const Argument::Validator &validator);
        ~ArgumentData();

        SymbolData *clone() const override;

    public:
        std::vector<std::string> expectedValues;
        Value defaultValue;
        bool required;
        std::string displayName;
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

        std::vector<Argument> arguments;
        std::unordered_map<std::string, size_t> argumentNameIndexes;
    };

}

#endif // ARGUMENT_P_H
