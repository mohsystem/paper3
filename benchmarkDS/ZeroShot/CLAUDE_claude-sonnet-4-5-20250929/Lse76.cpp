
#include <iostream>
#include <cstdlib>
#include <cstring>

struct Person {
    int status;
    char name[50];
    int age;
};

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        person->status = 0;
        memset(person->name, 0, sizeof(person->name));
        person->age = 0;
    }
    return person;
}

int main() {
    // Test case 1: Allocate single person
    Person* p1 = allocatePerson();
    if (p1) {
        std::cout << "Test 1 - Person status: " << p1->status << std::endl;
    }
    
    // Test case 2: Allocate multiple persons
    Person* p2 = allocatePerson();
    Person* p3 = allocatePerson();
    if (p2 && p3) {
        std::cout << "Test 2 - Person2 status: " << p2->status 
                  << ", Person3 status: " << p3->status << std::endl;
    }
    
    // Test case 3: Verify independence
    if (p1) {
        p1->status = 5;
        Person* p4 = allocatePerson();
        if (p4) {
            std::cout << "Test 3 - Person1 status: " << p1->status 
                      << ", Person4 status: " << p4->status << std::endl;
            free(p4);
        }
    }
    
    // Test case 4: Allocate and set properties
    Person* p5 = allocatePerson();
    if (p5) {
        strcpy(p5->name, "John");
        p5->age = 30;
        std::cout << "Test 4 - Person5 status: " << p5->status 
                  << ", Name: " << p5->name << std::endl;
        free(p5);
    }
    
    // Test case 5: Multiple allocations in loop
    for (int i = 0; i < 3; i++) {
        Person* p = allocatePerson();
        if (p) {
            std::cout << "Test 5 - Iteration " << i << " status: " << p->status << std::endl;
            free(p);
        }
    }
    
    // Clean up
    if (p1) free(p1);
    if (p2) free(p2);
    if (p3) free(p3);
    
    return 0;
}
