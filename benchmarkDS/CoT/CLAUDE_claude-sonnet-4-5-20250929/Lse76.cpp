
#include <iostream>
#include <cstdlib>
#include <new>

struct Person {
    int status;
    char name[50];
    int age;
};

Person* allocatePerson() {
    Person* person = nullptr;
    try {
        person = new Person();
        if (person != nullptr) {
            person->status = 0;
            person->name[0] = '\\0';
            person->age = 0;
        }
        return person;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

void freePerson(Person* person) {
    if (person != nullptr) {
        delete person;
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Test 1 - Person allocated, status: " << p1->status << std::endl;
    }
    
    // Test case 2: Verify status is 0
    Person* p2 = allocatePerson();
    if (p2 != nullptr) {
        std::cout << "Test 2 - Status is zero: " << (p2->status == 0) << std::endl;
    }
    
    // Test case 3: Multiple allocations
    Person* p3 = allocatePerson();
    if (p3 != nullptr) {
        std::cout << "Test 3 - Multiple allocation successful, status: " << p3->status << std::endl;
    }
    
    // Test case 4: Modify and verify independence
    Person* p4 = allocatePerson();
    if (p4 != nullptr) {
        p4->status = 5;
        std::cout << "Test 4 - Modified status: " << p4->status << std::endl;
    }
    
    // Test case 5: Verify new allocation is still 0
    Person* p5 = allocatePerson();
    if (p5 != nullptr) {
        std::cout << "Test 5 - New allocation status still 0: " << (p5->status == 0) << std::endl;
    }
    
    // Clean up
    freePerson(p1);
    freePerson(p2);
    freePerson(p3);
    freePerson(p4);
    freePerson(p5);
    
    return 0;
}
