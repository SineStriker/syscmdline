#ifndef HASH_HPP
#define HASH_HPP

#include <utility>
#include <functional>

#include "../capi/hash.h"

namespace SysCmdLine {

    struct HashHandler {
        void *(*construct)(const void *);
        void *(*move)(void *);
        void (*destroy)(void *);
    };

    template <class K, class T>
    class Hash {
    public:
        typedef K key_type;
        typedef T mapped_type;
        typedef int size_type;

        Hash()
            : _key_handler({
                  [](const void *t) -> void * {
                      return new K(*reinterpret_cast<const K *>(t)); //
                  },
                  [](void *t) -> void * {
                      return new K(std::move(*reinterpret_cast<K *>(t))); //
                  },
                  [](void *t) {
                      delete reinterpret_cast<K *>(t); //
                  },
              }),
              _value_handler({[](const void *t) -> void * {
                                  return new T(*reinterpret_cast<const T *>(t)); //
                              },
                              [](void *t) -> void * {
                                  return new T(std::move(*reinterpret_cast<T *>(t))); //
                              },
                              [](void *t) {
                                  delete reinterpret_cast<T *>(t); //
                              }}) {
            arg_hashtable_init(
                &_hashtable, 10,
                [](const void *key) {
                    return unsigned((std::hash<K>()(*reinterpret_cast<const K *>(key)))); //
                },
                [](const void *a, const void *b) -> int {
                    return *reinterpret_cast<const K *>(a) == *reinterpret_cast<const K *>(b); //
                });
        }

        ~Hash() {
            for (Iterator it = begin(); it != end(); ++it) {
                _key_handler.destroy(const_cast<K *>(&it.key()));
                _value_handler.destroy(&it.value());
            }
            arg_hashtable_fini(&_hashtable);
        }

        void insert(const K &key, const T &value) {
            arg_hashtable_insert(&_hashtable, _key_handler.construct(&key),
                                 _value_handler.construct(&value));
        }

        void insert(const K &key, T &&value) {
            arg_hashtable_insert(&_hashtable, _key_handler.construct(&key),
                                 _value_handler.move(&value));
        }

        void insert(K &&key, const T &value) {
            arg_hashtable_insert(&_hashtable, _key_handler.move(&key),
                                 _value_handler.construct(&value));
        }

        void insert(K &&key, T &&value) {
            arg_hashtable_insert(&_hashtable, _key_handler.move(&key), _value_handler.move(&value));
        }

        T *search(const K &key) {
            return reinterpret_cast<T *>(arg_hashtable_search(&_hashtable, &key));
        }

        const T *search(const K &key) const {
            return reinterpret_cast<const T *>(arg_hashtable_search(&_hashtable, &key));
        }

        class Iterator {
        public:
            typedef T value_type;
            typedef T *pointer;
            typedef T &reference;
            using iterator_category = std::forward_iterator_tag;

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

            const K &key() const {
                return *reinterpret_cast<K *>(
                    arg_hashtable_itr_key(const_cast<arg_hashtable_itr_t *>(&it)));
            }

            T &value() const {
                return *reinterpret_cast<T *>(
                    arg_hashtable_itr_value(const_cast<arg_hashtable_itr_t *>(&it)));
            }

            inline T &operator*() const {
                return value();
            }
            inline T *operator->() const {
                return &value();
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
            typedef T value_type;
            typedef T *pointer;
            typedef T &reference;
            using iterator_category = std::forward_iterator_tag;

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

            const K &key() const {
                return *reinterpret_cast<K *>(
                    arg_hashtable_itr_key(const_cast<arg_hashtable_itr_t *>(&it)));
            }

            const T &value() const {
                return *reinterpret_cast<T *>(const_cast<arg_hashtable_itr_t *>(&it));
            }

            inline T &operator*() const {
                return value();
            }

            inline T *operator->() const {
                return &value();
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

        ConstIterator find(const K &key) const {
            ConstIterator it;
            if (arg_hashtable_itr_search(&it.it, const_cast<arg_hashtable_t *>(&_hashtable),
                                         &key) == 0)
                it.it.h = nullptr;
            return it;
        }

        Iterator find(const K &key) {
            Iterator it;
            if (arg_hashtable_itr_search(&it.it, &_hashtable, &key) == 0)
                it.it.h = nullptr;
            return it;
        }

        Iterator erase(const ConstIterator &it) {
            _key_handler.destroy(const_cast<K *>(&it.key()));
            _value_handler.destroy(&it.value());
            if (arg_hashtable_itr_remove(&_hashtable, &it.it) == 0)
                return Iterator();
            Iterator res;
            res.it = it.it;
            return res;
        }

    protected:
        HashHandler _key_handler, _value_handler;
        arg_hashtable_t _hashtable;
    };

}

#endif // HASH_HPP
