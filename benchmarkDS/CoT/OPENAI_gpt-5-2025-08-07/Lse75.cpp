/*
Chain-of-Through Process:
1) Problem understanding: Provide an allocation function that returns a pointer to a newly allocated Person with status = 0.
2) Security requirements: Handle allocation failures safely; avoid memory leaks.
3) Secure coding generation: Use std::unique_ptr with nothrow new to prevent leaks and handle failures.
4) Code review: No raw new/delete leaks; check pointer before use.
5) Secure code output: Final code with 5 test cases and safe memory management.
*/

#include <iostream>
#include <memory>
#include <new>

struct Person {
    int status;
};

// Secure allocation: returns a unique_ptr<Person> to the beginning of allocated block; empty on failure.
std::unique_ptr<Person> allocate_person() {
    std::unique_ptr<Person> p{new (std::nothrow) Person()};
    if (!p) {
        return {};
    }
    // Initialize status to 0
    p->status = 0;
    return p;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        auto p = allocate_person();
        if (!p) {
            std::cout << "Test " << i << ": Allocation failed (null)." << std::endl;
        } else {
            std::cout << "Test " << i << ": Person allocated, status = " << p->status << std::endl;
        }
    }
    return 0;
}