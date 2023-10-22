#ifndef SYMBOL_P_H
#define SYMBOL_P_H

#include "symbol.h"

namespace SysCmdLine {

    class SymbolData : public SharedData {
    public:
        SymbolData(Symbol::SymbolType type, const std::string &name = {},
                   const std::string &desc = {});
        ~SymbolData();

        virtual SymbolData *clone() const = 0;

    public:
        Symbol::SymbolType type;
        std::string name;
        std::string desc;
        Symbol::HelpProvider helpProvider;
    };

}

#endif // SYMBOL_P_H
