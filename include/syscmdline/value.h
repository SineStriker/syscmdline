/****************************************************************************
 *
 * MIT License
 * 
 * Copyright (c) 2023 SineStriker
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ****************************************************************************/

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
