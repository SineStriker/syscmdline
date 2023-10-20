#include "symbol.h"
#include "symbol_p.h"

namespace SysCmdLine {

    SymbolData::SymbolData(Symbol::SymbolType type, const std::string &name,
                           const std::string &desc)
        : type(type), name(name), desc(desc) {
    }

    SymbolData::~SymbolData() {
    }

    Symbol::~Symbol() {
    }

    Symbol::SymbolType Symbol::type() const {
        return d_ptr->type;
    }

    std::string Symbol::name() const {
        return d_ptr->name;
    }

    void Symbol::setName(const std::string &name) {
        d_ptr->name = name;
    }

    std::string Symbol::description() const {
        return d_ptr->desc;
    }

    void Symbol::setDescription(const std::string &desc) {
        d_ptr->desc = desc;
    }

    Symbol::Symbol(SymbolData *d) : d_ptr(d) {
    }

}