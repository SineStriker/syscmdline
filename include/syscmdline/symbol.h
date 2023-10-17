#ifndef SYMBOL_H
#define SYMBOL_H

#include <string>
#include <vector>

#include <syscmdline/symbol.h>

namespace SysCmdLine {

    enum SymbolType {
        ST_Command,
        ST_Option,
        ST_Argument,
    };

    template <class T>
    class BasicSymbol {
    public:
        using value_type = T;
        using string_type = std::basic_string<T>;

        BasicSymbol(SymbolType type, const string_type &name = {}, const string_type &desc = {})
            : _type(type), _name(name), _desc(desc){};
        virtual ~BasicSymbol() = default;

    public:
        SymbolType type() const {
            return _type;
        }

        string_type name() const {
            return _name;
        }

        void setName(const string_type &name) {
            _name = name;
        }

        string_type description() const {
            return _desc;
        }

        void setDescription(const string_type &desc) {
            _desc = desc;
        }

    private:
        SymbolType _type;
        string_type _name;
        string_type _desc;
    };

}

#endif // SYMBOL_H
