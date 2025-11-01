#include <iostream>
#include <cstdlib>

struct Person {
    int status;
};

// Allocates a block of memory of size sizeof(Person) and returns pointer to it.
// Sets status to 0.
Person* allocate_person() {
    Person* p = static_cast<Person*>(std::malloc(sizeof(Person)));
    if (p) {
        p->status = 0;
    }
    return p;
}

int main() {
    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        Person* p = allocate_person();
        if (p) {
            std::cout << "Person{status=" << p->status << "}" << std::endl;
            std::free(p);
        } else {
            std::cout << "Allocation failed" << std::endl;
        }
    }
    return 0;
}