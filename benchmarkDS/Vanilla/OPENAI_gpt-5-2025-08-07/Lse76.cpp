#include <iostream>
#include <cstdlib>

struct Person {
    int status;
};

Person* allocate_person() {
    Person* p = static_cast<Person*>(std::malloc(sizeof(Person)));
    if (p != nullptr) {
        p->status = 0;
    }
    return p;
}

int main() {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        Person* p = allocate_person();
        if (p) {
            std::cout << p->status << std::endl;
            std::free(p);
        } else {
            std::cout << "allocation_failed" << std::endl;
        }
    }
    return 0;
}