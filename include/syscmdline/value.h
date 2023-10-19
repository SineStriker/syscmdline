#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <vector>

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
        Value(const char *ch, int size = -1);
        ~Value();

        Value(const Value &other);
        Value(Value &&other) noexcept;
        Value &operator=(const Value &other);
        Value &operator=(Value &&other) noexcept;

        inline Type type() const;
        bool isEmpty() const;

        int toInt() const;
        double toDouble() const;
        std::string toString() const;

        bool operator==(const Value &other) const;
        bool operator!=(const Value &other) const;

        static Value fromString(const std::string &s, Type type);
        static std::vector<std::string> toStringList(const std::vector<Value> &values);

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
