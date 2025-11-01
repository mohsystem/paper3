#include <iostream>
#include <cstdlib>

// Step 1: Define struct person with a status field.
struct person {
    int status;
};

// Step 3: Secure allocation using malloc(sizeof(person)), returning pointer to the block.
// Sets the status field to 0 explicitly.
person* allocate_person() {
    // Step 2: Security - check allocation result.
    void* mem = std::malloc(sizeof(person));
    if (!mem) {
        return nullptr;
    }
    person* p = static_cast<person*>(mem);
    // Step 3: Initialize fields securely.
    p->status = 0;
    return p;
}

// Step 5: Main with 5 test cases.
int main() {
    person* p1 = allocate_person();
    person* p2 = allocate_person();
    person* p3 = allocate_person();
    person* p4 = allocate_person();
    person* p5 = allocate_person();

    if (!p1 || !p2 || !p3 || !p4 || !p5) {
        std::cerr << "Allocation failed in one of the test cases.\n";
        std::free(p1);
        std::free(p2);
        std::free(p3);
        std::free(p4);
        std::free(p5);
        return 1;
    }

    std::cout << "Test1 status: " << p1->status << "\n";
    std::cout << "Test2 status: " << p2->status << "\n";
    std::cout << "Test3 status: " << p3->status << "\n";
    std::cout << "Test4 status: " << p4->status << "\n";
    std::cout << "Test5 status: " << p5->status << "\n";

    // Cleanup to prevent memory leaks.
    std::free(p1);
    std::free(p2);
    std::free(p3);
    std::free(p4);
    std::free(p5);

    return 0;
}