#include <iostream>
#include <new>

struct Person {
    int status;
};

Person* allocate_person() {
    // Allocate a block of memory for Person and set status to 0
    Person* p = new (std::nothrow) Person;
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
        if (!p) {
            std::cerr << "Allocation failed\n";
            return 1;
        }
        std::cout << "Test " << i << ": status=" << p->status << "\n";
        delete p;
    }
    return 0;
}