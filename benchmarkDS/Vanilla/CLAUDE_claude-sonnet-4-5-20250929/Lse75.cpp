
#include <iostream>
#include <cstdlib>
#include <cstring>

typedef struct {
    int status;
    char name[50];
    int age;
} Person;

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
    // Test case 1: Allocate a person and check status
    Person* p1 = allocatePerson();
    std::cout << "Test 1 - Status: " << p1->status << std::endl;
    
    // Test case 2: Allocate another person and verify independence
    Person* p2 = allocatePerson();
    p2->status = 1;
    std::cout << "Test 2 - P1 Status: " << p1->status << ", P2 Status: " << p2->status << std::endl;
    
    // Test case 3: Allocate third person
    Person* p3 = allocatePerson();
    std::cout << "Test 3 - Status: " << p3->status << std::endl;
    
    // Test case 4: Allocate fourth person
    Person* p4 = allocatePerson();
    std::cout << "Test 4 - Status: " << p4->status << std::endl;
    
    // Test case 5: Allocate fifth person
    Person* p5 = allocatePerson();
    std::cout << "Test 5 - Status: " << p5->status << std::endl;
    
    // Free allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
