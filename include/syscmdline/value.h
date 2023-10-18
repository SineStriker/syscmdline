#ifndef VALUE_H
#define VALUE_H

#include <string>

#include <syscmdline/global.h>

namespace SysCmdLine {

    class SYSCMDLINE_EXPORT Value {
    public:
        enum Type {
            Null,
            Int,
            Double,
            String,
        };

        Value(Type type = Null);
        Value(int i);
        Value(double d);
        Value(const std::string &s);
        ~Value();

        Value(const Value &other);
        Value(Value &&other) noexcept;
        Value &operator=(const Value &other);
        Value &operator=(Value &&other) noexcept;

        inline Type type() const;
        
        int toInt() const;
        double toDouble() const;
        std::string toString() const;

        bool operator==(const Value &other) const;
        bool operator!=(const Value &other) const;

    protected:
        union {
            int i;
            double d;
            std::string *s;
        } data;
        Type _type;
    };

    inline Value::Type Value::type() const {
        return _type;
    }

}

#endif // VALUE_H
