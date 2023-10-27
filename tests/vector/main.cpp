#include <iostream>
#include <unordered_map>
#include <vector>

#include "../../src/cxxxapi/vector.hpp"

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
    SysCmdLine::Vector<MyClass> vec;
    for (int i = 0; i < 20; ++i) {
        vec.append(i);
    }
    printf("%d\n", vec.size());

    
    // std::vector<MyClass> vec;
    // for (int i = 0; i < 20; ++i) {
    //     vec.push_back(i);
    // }
    // printf("%lld\n", vec.size());


    // vector_t vec;
    // vector_init(&vec);
    // for (int i = 0; i < 20; ++i) {
    //     vector_push_back(&vec, new MyClass(i));
    // }

    // for (int i = 0; i < vec.size; ++i) {
    //     delete reinterpret_cast<MyClass *>(vec.data[i]);
    // }
    // vector_fini(&vec);
    return 0;
}