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
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->type;
    }

    std::string Symbol::name() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->name;
    }

    void Symbol::setName(const std::string &name) {
        d_ptr->name = name;
    }

    std::string Symbol::description() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->desc;
    }

    void Symbol::setDescription(const std::string &desc) {
        SYSCMDLINE_GET_DATA(Symbol);
        d->desc = desc;
    }

    Symbol::HelpProvider Symbol::helpProvider() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->helpProvider;
    }

    void Symbol::setHelpProvider(const Symbol::HelpProvider &helpProvider) {
        SYSCMDLINE_GET_DATA(Symbol);
        d->helpProvider = helpProvider;
    }

    std::string Symbol::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        SYSCMDLINE_GET_DATA(const Symbol);
        if (d->helpProvider)
            return d->helpProvider(this, pos, displayOptions, extra);
        return {};
    }

    Symbol::Symbol(SymbolData *d) : d_ptr(d) {
    }

}