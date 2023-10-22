#include "symbol.h"
#include "symbol_p.h"

namespace SysCmdLine {

    /*!
        \namespace SysCmdLine
        \brief Library namespace.
    */

    SymbolData::SymbolData(Symbol::SymbolType type, const std::string &name,
                           const std::string &desc)
        : type(type), name(name), desc(desc) {
    }

    SymbolData::~SymbolData() {
    }

    /*!
        \class Symbol
        \brief Basic class of command line arguments instances.
    */

    /*!
        Destructor.
    */
    Symbol::~Symbol() {
    }

    /*!
        Returns the symbol type.
    */
    Symbol::SymbolType Symbol::type() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->type;
    }

    /*!
        Returns the symbol name.
    */
    std::string Symbol::name() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->name;
    }

    /*!
        Sets the symbol name.
    */
    void Symbol::setName(const std::string &name) {
        d_ptr->name = name;
    }

    /*!
        Sets the symbol description.
    */
    std::string Symbol::description() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->desc;
    }

    /*!
        Sets the symbol description.
    */
    void Symbol::setDescription(const std::string &desc) {
        SYSCMDLINE_GET_DATA(Symbol);
        d->desc = desc;
    }

    /*!
        Returns the symbol help provider.
    */
    Symbol::HelpProvider Symbol::helpProvider() const {
        SYSCMDLINE_GET_DATA(const Symbol);
        return d->helpProvider;
    }

    /*!
        Sets the symbol help provider.
    */
    void Symbol::setHelpProvider(const Symbol::HelpProvider &helpProvider) {
        SYSCMDLINE_GET_DATA(Symbol);
        d->helpProvider = helpProvider;
    }

    /*!
        Returns the help text of given position.
    */
    std::string Symbol::helpText(Symbol::HelpPosition pos, int displayOptions, void *extra) const {
        SYSCMDLINE_GET_DATA(const Symbol);
        if (d->helpProvider)
            return d->helpProvider(this, pos, displayOptions, extra);
        return {};
    }

    /*!
        \internal
    */
    Symbol::Symbol(SymbolData *d) : d_ptr(d) {
    }

}