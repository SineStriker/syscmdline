#ifndef MAP_P_H
#define MAP_P_H

#include <map>
#include <string>
#include <vector>

namespace SysCmdLine {

    using StringList = std::vector<std::string>;

    using IntList = std::vector<int>;

    union Ele {
        int i;
        size_t s;
        IntList *il;
        StringList *sl;
        void *p;
    };

    // Avoiding template specialization greatly helps to reduce the binary size,
    // so we only use this map in the library implementation.
    using GenericMap = std::map<std::string, Ele>;

    struct StringListMapWrapper {
        StringListMapWrapper() = default;

        StringListMapWrapper(const StringListMapWrapper &other) {
            for (const auto &pair : other.data) {
                data.insert(std::make_pair(pair.first, Ele{.sl = new StringList(*pair.second.sl)}));
            }
        }

        ~StringListMapWrapper() {
            for (const auto &pair : std::as_const(data)) {
                delete pair.second.sl;
            }
        }

        GenericMap data;
    };

    struct IntListMapWrapper {
        IntListMapWrapper() = default;

        IntListMapWrapper(const IntListMapWrapper &other) {
            for (const auto &pair : other.data) {
                data.insert(std::make_pair(pair.first, Ele{.il = new IntList(*pair.second.il)}));
            }
        }

        ~IntListMapWrapper() {
            for (const auto &pair : std::as_const(data)) {
                delete pair.second.il;
            }
        }

        GenericMap data;
    };

}

#endif // MAP_P_H
