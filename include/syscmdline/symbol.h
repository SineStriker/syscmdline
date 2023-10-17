#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>

namespace SysCmdLine {

    class Symbol {
    public:
        enum SymbolType {
            ST_Command,
            ST_Option,
            ST_Argument,
        };

        Symbol(SymbolType type, const std::string &name = {}, const std::string &desc = {});
        virtual ~Symbol();

    public:
        inline SymbolType type() const;

        inline std::string name() const;
        inline void setName(const std::string &name);

        inline std::string description() const;
        inline void setDescription(const std::string &desc);

    protected:
        SymbolType _type;
        std::string _name;
        std::string _desc;
    };

    inline Symbol::SymbolType Symbol::type() const {
        return _type;
    }

    inline std::string Symbol::name() const {
        return _name;
    }

    inline void Symbol::setName(const std::string &name) {
        _name = name;
    }

    inline std::string Symbol::description() const {
        return _desc;
    }

    inline void Symbol::setDescription(const std::string &desc) {
        _desc = desc;
    }

}

#endif // SYMBOL_H
