#include "symbol.h"

namespace SysCmdLine {

    Symbol::Symbol(SymbolType type, const std::string &name, const std::string &desc)
        : _type(type), _name(name), _desc(desc) {
    }

    Symbol::~Symbol() {
    }

}