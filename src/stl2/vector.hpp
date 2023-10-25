#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <utility>

#include "./c/vector.h"

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
            for (int i = 0; i < _vec.size; ++i) {
                _handler.destroy(_vec.data[i]);
            }
            vector_free(&_vec);
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

        void clear() {
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