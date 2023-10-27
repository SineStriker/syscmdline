#include <iostream>
#include <unordered_map>

#include "../../src/cxxxapi/hash.hpp"

class MyClass {
public:
    MyClass(int a = 0) : a(a) {
        std::cout << "construct" << a << std::endl;
    }

    MyClass(const MyClass &other) : a(other.a) {
        std::cout << "copy construct" << a << std::endl;
    }

    MyClass(MyClass &&other) : a(other.a) {
        std::cout << "move construct" << a << std::endl;
    }

    ~MyClass() {
        std::cout << "destruct" << a << std::endl;
    }
    int a;
};

int main(int argc, char *argv[]) {
    SysCmdLine::Hash<std::string, MyClass> hash;
    hash.insert("123", 3);
    hash.insert("456", 4);

    auto it = hash.find("456");
    printf("%d\n", it->a);

    SysCmdLine::Hash<std::string, int> hash2;
    hash.insert("123", 3);
    hash.insert("456", 4);

    auto it2 = hash2.find("456");
    printf("%d\n", it2.value());

    // std::unordered_map<std::string, MyClass> hash;
    // hash.insert(std::make_pair("123", 3));
    // hash.insert(std::make_pair("456", 4));

    // auto it = hash.find("456");
    // printf("%d\n", it->second.a);

    // std::unordered_map<std::string, int> hash2;
    // hash.insert(std::make_pair("123", 3));
    // hash.insert(std::make_pair("456", 4));

    // auto it2 = hash2.find("456");
    // printf("%d\n", it2->second);

    // // Create
    // arg_hashtable_t hash;
    // arg_hashtable_init(
    //     &hash, 10,
    //     [](const void *key) -> unsigned int {
    //         static auto _cmp = std::hash<std::string>();
    //         return _cmp(*reinterpret_cast<const std::string *>(key));
    //     },
    //     [](const void *a, const void *b) -> int {
    //         return *reinterpret_cast<const std::string *>(a) ==
    //                *reinterpret_cast<const std::string *>(b);
    //     });

    // arg_hashtable_insert(&hash, new std::string("123"), new MyClass(3));
    // arg_hashtable_insert(&hash, new std::string("456"), new MyClass(4));

    // // Search
    // {
    //     arg_hashtable_itr_t it;
    //     std::string key("123");
    //     arg_hashtable_itr_search(&it, &hash, &key);
    //     printf("%d\n", reinterpret_cast<MyClass *>(it.e->v)->a);
    // }
    // {
    //     arg_hashtable_itr_t it;
    //     std::string key("456");
    //     arg_hashtable_itr_search(&it, &hash, &key);
    //     printf("%d\n", reinterpret_cast<MyClass *>(it.e->v)->a);
    // }

    // // Destroy
    // {
    //     arg_hashtable_itr_t it;
    //     arg_hashtable_itr_init(&hash, &it);
    //     while (it.e) {
    //         delete reinterpret_cast<std::string *>(it.e->k);
    //         delete reinterpret_cast<MyClass *>(it.e->v);
    //         arg_hashtable_itr_advance(&it);
    //     }
    // }

    return 0;
}