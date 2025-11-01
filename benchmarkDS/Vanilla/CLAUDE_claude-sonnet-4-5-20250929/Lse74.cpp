
#include <iostream>
#include <cstdlib>
#include <cstring>

struct Person {
    int status;
    char name[50];
    int age;
};

Person* allocatePerson() {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p != NULL) {
        p->status = 0;
        memset(p->name, 0, sizeof(p->name));
        p->age = 0;
    }
    return p;
}

int main() {
    // Test case 1: Allocate a person and verify status is 0
    Person* person1 = allocatePerson();
    std::cout << "Test 1 - Status: " << person1->status << std::endl;
    
    // Test case 2: Allocate another person and verify status is 0
    Person* person2 = allocatePerson();
    std::cout << "Test 2 - Status: " << person2->status << std::endl;
    
    // Test case 3: Modify status and allocate new person
    person1->status = 5;
    Person* person3 = allocatePerson();
    std::cout << "Test 3 - Person1 Status: " << person1->status 
              << ", Person3 Status: " << person3->status << std::endl;
    
    // Test case 4: Verify multiple allocations
    Person* person4 = allocatePerson();
    Person* person5 = allocatePerson();
    std::cout << "Test 4 - Person4 Status: " << person4->status 
              << ", Person5 Status: " << person5->status << std::endl;
    
    // Test case 5: Verify pointers are different
    std::cout << "Test 5 - Are person1 and person2 different objects? " 
              << (person1 != person2 ? "true" : "false") << std::endl;
    
    // Free allocated memory
    free(person1);
    free(person2);
    free(person3);
    free(person4);
    free(person5);
    
    return 0;
}
