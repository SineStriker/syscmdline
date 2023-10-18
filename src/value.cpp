#include "value.h"

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

}