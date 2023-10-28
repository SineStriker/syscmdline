#ifndef MAP_P_H
#define MAP_P_H

#include <map>
#include <string>
#include <thread>

namespace SysCmdLine {

    using StringList = std::vector<std::string>;

    using IntList = std::vector<int>;

    using StringMap = std::map<std::string, size_t>;

    using SSizeMap = std::map<size_t, size_t>;

    template <class T, class K, class... Args>
    inline void map_insert(std::map<K, size_t> &map, const std::string &key, Args &&...args) {
        map.insert(std::make_pair(key, size_t(new T(std::forward<Args>(args)...))));
    }

    template <class K, class T>
    inline const T &map_get(const std::map<K, T> &map, const K &key) {
        return map.find(key)->second;
    }

    template <class K>
    inline const void *map_search(const std::map<K, size_t> &map, const K &key) {
        auto it = map.find(key);
        if (it == map.end())
            return nullptr;
        return reinterpret_cast<const void *>(it->second);
    }

    template <class T, class K>
    inline T *map_search(std::map<K, size_t> &map, const K &key) {
        auto it = map.find(key);
        if (it == map.end())
            return nullptr;
        return reinterpret_cast<T *>(it->second);
    }

    template <class T, class K>
    inline void map_deleteAll(const std::map<K, size_t> &map) {
        for (const auto &val : map) {
            delete reinterpret_cast<T *>(val.second);
        }
    }

    template <class T, class K>
    inline StringMap map_copy(const std::map<K, size_t> &map) {
        StringMap res;
        for (auto it = map.begin(); it != map.end(); ++it) {
            res.insert(
                std::make_pair(it->first, size_t(new T(*reinterpret_cast<const T *>(it->second)))));
        }
        return res;
    }

}

#endif // MAP_P_H
