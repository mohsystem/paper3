
#include <iostream>
#include <cstdlib>
#include <new>

struct Person {
    int status;
};

Person* allocatePerson() {
    Person* person = nullptr;
    
    try {
        person = new(std::nothrow) Person;
        
        if (person == nullptr) {
            std::cerr << "Memory allocation failed" << std::endl;
            return nullptr;
        }
        
        person->status = 0;
        return person;
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Test 1 - Status: " << p1->status << std::endl;
    }
    
    // Test case 2: Multiple allocations
    Person* p2 = allocatePerson();
    if (p2 != nullptr) {
        std::cout << "Test 2 - Status: " << p2->status << std::endl;
    }
    
    // Test case 3: Verify independent objects
    Person* p3 = allocatePerson();
    if (p3 != nullptr) {
        p3->status = 5;
        std::cout << "Test 3 - Modified status: " << p3->status << std::endl;
    }
    
    // Test case 4: Check original unchanged
    if (p1 != nullptr) {
        std::cout << "Test 4 - Original status: " << p1->status << std::endl;
    }
    
    // Test case 5: Multiple allocations in loop
    std::cout << "Test 5 - Allocating 5 persons:" << std::endl;
    Person* persons[5];
    for (int i = 0; i < 5; i++) {
        persons[i] = allocatePerson();
        if (persons[i] != nullptr) {
            std::cout << "  Person " << (i+1) << " status: " << persons[i]->status << std::endl;
        }
    }
    
    // Clean up memory
    delete p1;
    delete p2;
    delete p3;
    for (int i = 0; i < 5; i++) {
        if (persons[i] != nullptr) {
            delete persons[i];
        }
    }
    
    return 0;
}
