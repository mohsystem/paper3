
#include <iostream>
#include <cstdlib>
#include <memory>

class Person {
private:
    int status;

public:
    Person() : status(0) {}
    
    int getStatus() const {
        return status;
    }
    
    void setStatus(int s) {
        status = s;
    }
};

Person* allocatePerson() {
    try {
        Person* person = new(std::nothrow) Person();
        if (person == nullptr) {
            std::cerr << "Memory allocation failed" << std::endl;
            return nullptr;
        }
        return person;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

void deallocatePerson(Person* person) {
    if (person != nullptr) {
        delete person;
        person = nullptr;
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Test 1 - Status: " << p1->getStatus() << std::endl;
    }
    
    // Test case 2: Multiple allocations
    Person* p2 = allocatePerson();
    if (p2 != nullptr) {
        std::cout << "Test 2 - Status: " << p2->getStatus() << std::endl;
    }
    
    // Test case 3: Verify independence
    if (p1 != nullptr) {
        p1->setStatus(5);
    }
    if (p2 != nullptr) {
        std::cout << "Test 3 - P2 Status (should be 0): " << p2->getStatus() << std::endl;
    }
    
    // Test case 4: Another allocation
    Person* p3 = allocatePerson();
    if (p3 != nullptr) {
        std::cout << "Test 4 - Status: " << p3->getStatus() << std::endl;
    }
    
    // Test case 5: Verify initialization
    Person* p4 = allocatePerson();
    if (p4 != nullptr) {
        std::cout << "Test 5 - Status: " << p4->getStatus() << std::endl;
    }
    
    // Clean up
    deallocatePerson(p1);
    deallocatePerson(p2);
    deallocatePerson(p3);
    deallocatePerson(p4);
    
    return 0;
}
