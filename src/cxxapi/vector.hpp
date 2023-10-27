#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>

#include "../capi/vector.h"

namespace SysCmdLine {

    struct VectorHandler {
        void *(*construct)(const void *);
        void *(*move)(void *);
        void (*destroy)(void *);
    };

    template <class T>
    class Vector {
    public:
        Vector()
            : _handler({
                  [](const void *t) -> void * {
                      return new T(*reinterpret_cast<const T *>(t)); //
                  },
                  [](void *t) -> void * {
                      return new T(std::move(*reinterpret_cast<T *>(t))); //
                  },
                  [](void *t) {
                      delete reinterpret_cast<T *>(t); //
                  },
              }) {
            vector_init(&_vec);
        }

        ~Vector() {
            clear();
            vector_fini(&_vec);
        }

        Vector(const Vector &other) : Vector() {
            reserve(other.size());
            for (int i = 0; i < other.size(); ++i) {
                append(other[i]);
            }
        }

        Vector(Vector &&other) noexcept : Vector() {
            std::move(_vec, other._vec);
        }

        Vector &operator=(const Vector &other) {
            if (this == &other) {
                return *this;
            }
            clear();
            reserve(other.size());
            for (int i = 0; i < other.size(); ++i) {
                append(other[i]);
            }
            return *this;
        }

        Vector &operator=(Vector &&other) noexcept {
            std::move(_vec, other._vec);
        }

        int size() const {
            return _vec.size;
        }

        void reserve(int size) {
            vector_reserve(&_vec, size);
        }

        void resize(int size) {
            vector_resize(&_vec, size);
        }

        void append(const T &t) {
            vector_push_back(&_vec, _handler.construct(&t));
        }

        void append(T &&t) {
            vector_push_back(&_vec, _handler.move(&t));
        }

        void push_back(const T &t) {
            vector_push_back(&_vec, _handler.construct(&t));
        }

        void push_back(T &&t) {
            vector_push_back(&_vec, _handler.move(&t));
        }

        void clear() {
            for (int i = 0; i < _vec.size; ++i) {
                _handler.destroy(_vec.data[i]);
            }
            vector_clear(&_vec);
        }

        T &operator[](int index) {
            return *reinterpret_cast<T *>(_vec.data[index]);
        }

        const T &operator[](int index) const {
            return *reinterpret_cast<T *>(_vec.data[index]);
        }

    protected:
        VectorHandler _handler;
        vector_t _vec;
    };

}

#endif // VECTOR_HPP