
#include <iostream>
#include <cstdlib>
#include <new>

struct Person {
    int status;
    char name[50];
    int age;
};

Person* allocatePerson() {
    try {
        Person* person = new Person();
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
    std::cout << "Test Case 1: Basic allocation" << std::endl;
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Person allocated, status: " << p1->status << std::endl;
        freePerson(p1);
    }
    
    std::cout << "\\nTest Case 2: Multiple allocations" << std::endl;
    Person* p2 = allocatePerson();
    Person* p3 = allocatePerson();
    if (p2 != nullptr && p3 != nullptr) {
        std::cout << "Multiple persons allocated successfully" << std::endl;
        freePerson(p2);
        freePerson(p3);
    }
    
    std::cout << "\\nTest Case 3: Verify status initialization" << std::endl;
    Person* p4 = allocatePerson();
    if (p4 != nullptr) {
        std::cout << "Status is zero: " << (p4->status == 0) << std::endl;
        freePerson(p4);
    }
    
    std::cout << "\\nTest Case 4: Modify status" << std::endl;
    Person* p5 = allocatePerson();
    if (p5 != nullptr) {
        p5->status = 1;
        std::cout << "Modified status: " << p5->status << std::endl;
        freePerson(p5);
    }
    
    std::cout << "\\nTest Case 5: Null check" << std::endl;
    Person* p6 = allocatePerson();
    std::cout << "Allocation result: " << (p6 != nullptr ? "Success" : "Failed") << std::endl;
    freePerson(p6);
    
    return 0;
}
