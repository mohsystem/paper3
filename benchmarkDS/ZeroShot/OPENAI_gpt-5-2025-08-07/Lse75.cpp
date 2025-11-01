#include <iostream>
#include <new>

struct Person {
    int status;
};

Person* allocate_person() {
    // Allocates a new Person and sets status to 0.
    Person* p = new (std::nothrow) Person();
    if (!p) {
        return nullptr;
    }
    p->status = 0;
    return p;
}

int main() {
    // 5 test cases
    for (int i = 1; i <= 5; ++i) {
        Person* p = allocate_person();
        if (p == nullptr) {
            std::cerr << "Allocation failed at test " << i << std::endl;
        } else {
            std::cout << "Test " << i << ": status=" << p->status << std::endl;
            delete p;
            p = nullptr;
        }
    }
    return 0;
}