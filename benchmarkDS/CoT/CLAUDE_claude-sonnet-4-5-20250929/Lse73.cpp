
#include <iostream>
#include <cstring>
#include <new>

struct Person {
    char name[50];
    int age;
    
    Person() : age(0) {
        memset(name, 0, sizeof(name));
    }
    
    Person(const char* n, int a) : age(a) {
        memset(name, 0, sizeof(name));
        if (n != nullptr) {
            strncpy(name, n, sizeof(name) - 1);
            name[sizeof(name) - 1] = '\\0';
        }
    }
};

Person* allocatePerson() {
    try {
        Person* p = new Person();
        return p;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

Person* allocatePerson(const char* name, int age) {
    try {
        Person* p = new Person(name, age);
        return p;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

void deallocatePerson(Person* p) {
    if (p != nullptr) {
        delete p;
    }
}

int main() {
    // Test case 1: Basic allocation
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        strncpy(p1->name, "Alice", sizeof(p1->name) - 1);
        p1->age = 25;
        std::cout << "Test 1: Person{name='" << p1->name << "', age=" << p1->age << "}" << std::endl;
        deallocatePerson(p1);
    }
    
    // Test case 2: Allocation with parameters
    Person* p2 = allocatePerson("Bob", 30);
    if (p2 != nullptr) {
        std::cout << "Test 2: Person{name='" << p2->name << "', age=" << p2->age << "}" << std::endl;
        deallocatePerson(p2);
    }
    
    // Test case 3: Multiple allocations
    Person* p3 = allocatePerson("Charlie", 35);
    if (p3 != nullptr) {
        std::cout << "Test 3: Person{name='" << p3->name << "', age=" << p3->age << "}" << std::endl;
        deallocatePerson(p3);
    }
    
    // Test case 4: Null validation
    Person* p4 = allocatePerson("Diana", 28);
    if (p4 != nullptr) {
        std::cout << "Test 4: Person{name='" << p4->name << "', age=" << p4->age << "}" << std::endl;
        deallocatePerson(p4);
    } else {
        std::cout << "Test 4: Allocation failed" << std::endl;
    }
    
    // Test case 5: Zero age
    Person* p5 = allocatePerson("Eve", 0);
    if (p5 != nullptr) {
        std::cout << "Test 5: Person{name='" << p5->name << "', age=" << p5->age << "}" << std::endl;
        deallocatePerson(p5);
    }
    
    return 0;
}
