#ifndef HASH_HPP
#define HASH_HPP

#include <utility>
#include <functional>

#include "./c/hash.h"

namespace SysCmdLine {

    struct HashHandler {
        void *(*construct)(const void *);
        void *(*move)(void *);
        void (*destroy)(void *);
    };

    template <class T, class V>
    class Hash {
    public:
        typedef V mapped_type;
        typedef T key_type;
        typedef int size_type;

        Hash()
            : _key_handler({
                  [](const void *t) -> void * {
                      return new T(*reinterpret_cast<const T *>(t)); //
                  },
                  [](void *t) -> void * {
                      return new T(std::move(*reinterpret_cast<T *>(t))); //
                  },
                  [](void *t) {
                      delete reinterpret_cast<T *>(t); //
                  },
              }),
              _value_handler({[](const void *t) -> void * {
                                  return new V(*reinterpret_cast<const V *>(t)); //
                              },
                              [](void *t) -> void * {
                                  return new V(std::move(*reinterpret_cast<V *>(t))); //
                              },
                              [](void *t) {
                                  delete reinterpret_cast<V *>(t); //
                              }}) {
            arg_hashtable_init(
                &_hashtable, 10,
                [](const void *key) {
                    return int((*reinterpret_cast<const T *>(std::hash<T>()(key)))); //
                },
                [](const void *a, const void *b) -> int {
                    return *reinterpret_cast<const T *>(a) == *reinterpret_cast<const T *>(b); //
                });
        }

        ~Hash() {
            arg_hashtable_fini(&_hashtable);
        }

        void insert(const T &key, const V &value) {
            arg_hashtable_insert(&_hashtable, _key_handler.construct(&key),
                                 _key_handler.construct(&value));
        }

        void insert(const T &key, V &&value) {
            arg_hashtable_insert(&_hashtable, _key_handler.construct(&key),
                                 _key_handler.move(&value));
        }

        void insert(T &&key, const V &value) {
            arg_hashtable_insert(&_hashtable, _key_handler.move(&key),
                                 _key_handler.construct(&value));
        }

        void insert(T &&key, V &&value) {
            arg_hashtable_insert(&_hashtable, _key_handler.move(&key), _key_handler.move(&value));
        }

        V *search(const T &key) {
            return reinterpret_cast<V *>(arg_hashtable_search(&_hashtable, &key));
        }

        const V *search(const T &key) const {
            return reinterpret_cast<const V *>(arg_hashtable_search(&_hashtable, &key));
        }

        void remove(const T &key) const {
            arg_hashtable_remove(&_hashtable, &key);
        }

        class Iterator {
        public:
            Iterator() {
                it.h = nullptr;
            }

            Iterator operator++(int) {
                Iterator org = *this;
                if (arg_hashtable_itr_advance(&it) == 0)
                    return Iterator();
                return org;
            };

            Iterator &operator++() {
                if (arg_hashtable_itr_advance(&it) == 0)
                    it.h = nullptr;
                return *this;
            };

            const T &key() const {
                return *reinterpret_cast<T *>(arg_hashtable_itr_key(&it));
            }

            T &value() {
                return *reinterpret_cast<V *>(arg_hashtable_itr_value(&it));
            }

            operator bool() const {
                return it.h != nullptr;
            }

        private:
            Iterator(arg_hashtable_t *h) {
                arg_hashtable_itr_init(h, &it);
            }
            arg_hashtable_itr_t it;

            friend class Hash;
        };

        class ConstIterator {
        public:
            ConstIterator() {
                it.h = nullptr;
            }

            ConstIterator(const Iterator &other) {
                it = other.it;
            }

            ConstIterator operator++(int) {
                Iterator org = *this;
                if (arg_hashtable_itr_advance(&it) == 0)
                    return ConstIterator();
                return org;
            };

            ConstIterator &operator++() {
                if (arg_hashtable_itr_advance(&it) == 0)
                    it.h = nullptr;
                return *this;
            };

            const T &key() const {
                return *reinterpret_cast<T *>(arg_hashtable_itr_key(&it));
            }

            const V &value() {
                return *reinterpret_cast<V *>(arg_hashtable_itr_value(&it));
            }

            operator bool() const {
                return it.h != nullptr;
            }

        private:
            ConstIterator(const arg_hashtable_t *h) {
                arg_hashtable_itr_init(const_cast<arg_hashtable_t *>(h), &it);
            }
            arg_hashtable_itr_t it;

            friend class Hash;
        };

        Iterator begin() {
            return Iterator(&_hashtable);
        }

        Iterator end() {
            return Iterator();
        }

        ConstIterator begin() const {
            return ConstIterator(&_hashtable);
        }

        ConstIterator end() const {
            return ConstIterator();
        }

        ConstIterator find(const T &key) const {
            ConstIterator it;
            if (arg_hashtable_itr_search(&it.it, const_cast<arg_hashtable_t *>(&_hashtable),
                                         &key) == 0)
                it.it.h = nullptr;
            return it;
        }

        Iterator find(const T &key) {
            Iterator it;
            if (arg_hashtable_itr_search(&it.it, &_hashtable, &key) == 0)
                it.it.h = nullptr;
            return it;
        }

    protected:
        HashHandler _key_handler, _value_handler;
        arg_hashtable_t _hashtable;
    };

}

#include <string>

static int aa() {
    SysCmdLine::Hash<std::string, int> hash;

    for (const auto &item : hash) {
    }
}


#endif // HASH_HPP
