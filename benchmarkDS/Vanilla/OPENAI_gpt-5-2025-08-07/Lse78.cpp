#include <iostream>
#include <cstdlib>

struct Person {
    int status;
};

void* allocateMemory(std::size_t size) {
    return std::malloc(size);
}

Person* allocatePerson() {
    Person* p = static_cast<Person*>(std::malloc(sizeof(Person)));
    if (p) p->status = 0;
    return p;
}

int main() {
    std::size_t sizes[5] = {0, 1, 16, 1024, 65536};

    // 5 test cases for memory allocation
    for (std::size_t s : sizes) {
        void* mem = allocateMemory(s);
        std::cout << "Allocated memory pointer: " << mem << " size: " << s << "\n";
        std::free(mem);
    }

    // 5 test cases for person allocation
    for (int i = 0; i < 5; ++i) {
        Person* p = allocatePerson();
        std::cout << "Person status: " << (p ? p->status : -1) << "\n";
        std::free(p);
    }

    return 0;
}