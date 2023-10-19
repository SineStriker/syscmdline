#include "value.h"

#include <algorithm>

#include "strings.h"

namespace SysCmdLine {

    Value::Value(Value::Type type) : _type(type) {
        switch (type) {
            case Int:
                data.i = 0;
                break;
            case Double:
                data.d = 0;
                break;
            case String:
                data.s = new std::string();
                break;
            default:
                break;
        }
    }

    Value::Value(int i) : _type(Int) {
        data.i = i;
    }

    Value::Value(double d) : _type(Double) {
        data.d = d;
    }

    Value::Value(const std::string &s) : _type(String) {
        data.s = new std::string(s);
    }

    Value::Value(const char *ch, int size) : _type(String) {
        data.s = size >= 0 ? new std::string(ch, size) : new std::string(ch);
    }

    Value::~Value() {
        if (_type == String)
            delete data.s;
    }

    Value::Value(const Value &other) {
        _type = other._type;
        switch (_type) {
            case Int:
                data.i = other.data.i;
                break;
            case Double:
                data.d = other.data.d;
                break;
            case String:
                data.s = new std::string(*other.data.s);
                break;
            default:
                break;
        }
    }

    Value::Value(Value &&other) noexcept {
        _type = other._type;
        switch (_type) {
            case Int:
                data.i = other.data.i;
                break;
            case Double:
                data.d = other.data.d;
                break;
            case String:
                data.s = other.data.s;
                other.data.s = nullptr;
                break;
            default:
                break;
        }
    }

    Value &Value::operator=(const Value &other) {
        if (this == &other) {
            return *this;
        }

        if (_type == String) {
            delete data.s;
        }
        _type = other._type;
        switch (_type) {
            case Int:
                data.i = other.data.i;
                break;
            case Double:
                data.d = other.data.d;
                break;
            case String:
                data.s = new std::string(*other.data.s);
                break;
            default:
                break;
        }
        return *this;
    }

    Value &Value::operator=(Value &&other) noexcept {
        if (this == &other) {
            return *this;
        }
        std::swap(data, other.data);
        std::swap(_type, other._type);
        return *this;
    }

    bool Value::isEmpty() const {
        switch (_type) {
            case String:
                return data.s->empty();
            case Double:
                return data.d == 0;
            case Int:
                return data.i == 0;
            default:
                break;
        }
        return true;
    }

    int Value::toInt() const {
        return _type == Int ? data.i : int(toDouble());
    }

    double Value::toDouble() const {
        return _type == Double ? data.d : 0;
    }

    std::string Value::toString() const {
        switch (_type) {
            case String:
                return *data.s;
            case Double:
                return std::to_string(data.d);
            case Int:
                return std::to_string(data.i);
            default:
                break;
        }
        return {};
    }

    bool Value::operator==(const Value &other) const {
        if (_type != other._type)
            return false;
        switch (_type) {
            case Int:
                return data.i == other.data.i;
            case Double:
                return data.d == other.data.d;
            case String:
                return *data.s == *other.data.s;
            default:
                break;
        }
        return true;
    }

    bool Value::operator!=(const Value &other) const {
        return !(*this == other);
    }

    Value Value::fromString(const std::string &s, Value::Type type) {
        Value res;
        switch (type) {
            case Value::Int: {
                std::string::size_type idx;
                try {
                    int base = 10;
                    std::string s1 = s;
                    if (s.size() > 2 && s.front() == '0') {
                        switch (s.at(1)) {
                            case 'x':
                            case 'X':
                                s1 = s1.substr(2);
                                base = 16;
                                break;
                            case 'b':
                            case 'B':
                                s1 = s1.substr(2);
                                base = 2;
                                break;
                            case 'o':
                            case 'O':
                                s1 = s1.substr(2);
                                base = 8;
                                break;
                            case 'd':
                            case 'D':
                                s1 = s1.substr(2);
                                break;
                            default:
                                break;
                        }
                    }
                    res = std::stoi(s1, &idx, base);
                    if (idx < s1.size()) {
                        res = {};
                    }
                } catch (...) {
                }
                break;
            }

            case Value::Double: {
                std::string::size_type idx;
                try {
                    res = std::stod(s, &idx);
                    if (idx < s.size()) {
                        res = {};
                    }
                } catch (...) {
                }
                break;
            }

            case Value::String: {
                if (!s.empty()) {
                    res = s;
                }
                break;
            }
            default:
                break;
        }
        return res;
    }

    std::vector<std::string> Value::toStringList(const std::vector<Value> &values) {
        std::vector<std::string> res(values.size());
        std::transform(values.begin(), values.end(), res.begin(), [](const Value &val) {
            return val.toString(); //
        });
        return res;
    }

}