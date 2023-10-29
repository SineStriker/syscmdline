#include "value.h"

#include <algorithm>

#include "utils_p.h"

namespace SysCmdLine {

    static const char True_Literal[] = "true";
    static const char False_Literal[] = "false";

    Value::Value(Value::Type type) : _type(type) {
        switch (type) {
            case Bool:
                data.b = false;
                break;
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

    Value::~Value() {
        if (_type == String)
            delete data.s;
    }

    Value::Value(const Value &other) {
        _type = other._type;
        switch (_type) {
            case Bool:
                data.b = other.data.b;
                break;
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
            case Bool:
                data.b = other.data.b;
                break;
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
            case Bool:
                data.b = other.data.b;
                break;
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
            case Bool:
                return !data.b;
            case Int:
                return data.i == 0;
            case Double:
                return data.d == 0;
            case String:
                return data.s->empty();
            default:
                break;
        }
        return true;
    }

    bool Value::toBool() const {
        return _type == Bool ? data.b : bool(toInt());
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
            case Bool:
                return data.b ? True_Literal : False_Literal;
            default:
                break;
        }
        return {};
    }

    bool Value::operator==(const Value &other) const {
        if (_type != other._type)
            return false;
        switch (_type) {
            case Bool:
                return data.b == other.data.b;
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
            case Bool: {
                if (Utils::toLower(s) == True_Literal) {
                    res = true;
                } else if (Utils::toLower(s) == False_Literal) {
                    res = false;
                }
                break;
            }

            case Int: {
                std::string::size_type idx;
                try {
                    int base = 10;
                    std::string s1 = s;
                    int f = 1;
                    if (s1.front() == '+') {
                        s1 = s1.substr(1);
                    } else if (s1.front() == '-') {
                        f = -1;
                        s1 = s1.substr(1);
                    }
                    if (s1.size() > 2 && s1.front() == '0') {
                        switch (s1.at(1)) {
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
                    res = std::stoi(s1, &idx, base) * f;
                    if (idx < s1.size()) {
                        res = {};
                    }
                } catch (...) {
                }
                break;
            }

            case Double: {
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

            case String: {
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