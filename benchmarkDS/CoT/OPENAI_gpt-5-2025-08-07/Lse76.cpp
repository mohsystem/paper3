// Chain-of-Through process:
// 1) Problem understanding: Allocate a Person and set status to 0.
// 2) Security requirements: Use nothrow new; check for allocation failure.
// 3) Secure coding: Initialize fields explicitly; clean up memory to avoid leaks.
// 4) Code review: Checked for nullptr, prints only after validation.
// 5) Secure code output: Final code with 5 test cases and proper deallocation.

#include <iostream>
#include <new>

struct Person {
    int status;
};

Person* allocate_person() {
    Person* p = new (std::nothrow) Person;
    if (!p) {
        return nullptr;
    }
    p->status = 0;
    return p;
}

void free_person(Person* p) {
    delete p;
}

int main() {
    Person* people[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    for (int i = 0; i < 5; ++i) {
        people[i] = allocate_person();
    }

    for (int i = 0; i < 5; ++i) {
        if (people[i]) {
            std::cout << "Test " << (i + 1) << " - Person status: " << people[i]->status << "\n";
        } else {
            std::cout << "Test " << (i + 1) << " - Allocation failed\n";
        }
    }

    for (int i = 0; i < 5; ++i) {
        free_person(people[i]);
    }

    return 0;
}