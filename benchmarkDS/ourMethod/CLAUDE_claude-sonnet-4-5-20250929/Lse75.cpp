
#include <iostream>
#include <cstdlib>
#include <new>

struct Person {
    int status;
};

// Allocates a new Person with status initialized to 0
// Returns nullptr on allocation failure
Person* allocate_person() {
    try {
        // Allocate memory for Person struct
        Person* p = new(std::nothrow) Person;
        
        // Check if allocation succeeded
        if (p == nullptr) {
            return nullptr;
        }
        
        // Initialize status to 0
        p->status = 0;
        
        return p;
    } catch (const std::bad_alloc&) {
        // Return nullptr on allocation failure
        return nullptr;
    }
}

// Safely deallocate Person
void free_person(Person* p) {
    if (p != nullptr) {
        delete p;
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocate_person();
    if (p1 != nullptr) {
        std::cout << "Test 1 - Person allocated, status: " << p1->status << std::endl;
        free_person(p1);
    } else {
        std::cout << "Test 1 - Allocation failed" << std::endl;
    }
    
    // Test case 2: Verify status is 0
    Person* p2 = allocate_person();
    if (p2 != nullptr) {
        std::cout << "Test 2 - Status is zero: " << (p2->status == 0) << std::endl;
        free_person(p2);
    }
    
    // Test case 3: Multiple allocations
    Person* p3 = allocate_person();
    if (p3 != nullptr) {
        std::cout << "Test 3 - Multiple allocations work, status: " << p3->status << std::endl;
        free_person(p3);
    }
    
    // Test case 4: Verify independence of objects
    Person* p4 = allocate_person();
    Person* p5 = allocate_person();
    if (p4 != nullptr && p5 != nullptr) {
        p4->status = 1;
        std::cout << "Test 4 - Objects are independent: " << (p5->status == 0) << std::endl;
        free_person(p4);
        free_person(p5);
    }
    
    // Test case 5: Null check handling
    Person* p6 = allocate_person();
    std::cout << "Test 5 - Null check passed: " << (p6 != nullptr) << std::endl;
    free_person(p6);
    
    return 0;
}
