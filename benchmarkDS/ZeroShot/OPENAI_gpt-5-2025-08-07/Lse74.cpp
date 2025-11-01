#include <iostream>
#include <memory>
#include <new>

struct Person {
    int status;
};

// Allocates a new Person and ensures status is set to 0.
std::unique_ptr<Person> allocatePerson(int /*ignoredStatus*/) {
    std::unique_ptr<Person> p(new (std::nothrow) Person);
    if (!p) {
        return nullptr;
    }
    p->status = 0;
    return p;
}

int main() {
    int tests[5] = {-1, 0, 1, 42, 2147483647};
    for (int i = 0; i < 5; ++i) {
        auto p = allocatePerson(tests[i]);
        if (!p) {
            std::cout << "Test " << (i + 1) << ": allocation failed\n";
        } else {
            std::cout << "Test " << (i + 1) << ": input=" << tests[i] << " status=" << p->get()->status << "\n";
        }
    }
    return 0;
}