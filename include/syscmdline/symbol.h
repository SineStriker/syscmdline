#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

#include <syscmdline/global.h>
#include <syscmdline/shareddata.h>

namespace SysCmdLine {

    class SymbolData;

    class SYSCMDLINE_EXPORT Symbol {
        SYSCMDLINE_DECL_DATA(Symbol)
    public:
        enum SymbolType {
            ST_Command,
            ST_Option,
            ST_Argument,
        };

        virtual ~Symbol();

    public:
        SymbolType type() const;

        std::string name() const;
        void setName(const std::string &name);

        std::string description() const;
        void setDescription(const std::string &desc);

    protected:
        Symbol(SymbolData *d);

        SharedDataPointer<SymbolData> d_ptr;
    };

}

#endif // SYMBOL_H
