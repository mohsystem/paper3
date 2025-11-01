#include <iostream>
#include <cstdlib>

struct person {
    int status;
};

// Allocates a block of memory of size sizeof(person) and returns a pointer
person* allocate_person() {
    person* p = static_cast<person*>(std::malloc(sizeof(person)));
    if (p != nullptr) {
        p->status = 0;
    }
    return p;
}

int main() {
    // 5 test cases
    person* p1 = allocate_person();
    person* p2 = allocate_person();
    person* p3 = allocate_person();
    person* p4 = allocate_person();
    person* p5 = allocate_person();

    if (p1) std::cout << "p1->status = " << p1->status << "\n";
    if (p2) std::cout << "p2->status = " << p2->status << "\n";
    if (p3) std::cout << "p3->status = " << p3->status << "\n";
    if (p4) std::cout << "p4->status = " << p4->status << "\n";
    if (p5) std::cout << "p5->status = " << p5->status << "\n";

    std::free(p1);
    std::free(p2);
    std::free(p3);
    std::free(p4);
    std::free(p5);

    return 0;
}