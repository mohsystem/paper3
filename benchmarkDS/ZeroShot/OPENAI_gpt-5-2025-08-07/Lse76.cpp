#include <iostream>
#include <new>

struct Person {
    int status;
};

Person* allocate_person() noexcept {
    Person* p = new (std::nothrow) Person{};
    if (!p) return nullptr;
    p->status = 0;
    return p;
}

int main() {
    for (int i = 1; i <= 5; i++) {
        Person* p = allocate_person();
        if (!p) {
            std::cout << "Allocation failed\n";
        } else {
            std::cout << "Test " << i << ": status=" << p->status << "\n";
        }
        delete p;
    }
    return 0;
}