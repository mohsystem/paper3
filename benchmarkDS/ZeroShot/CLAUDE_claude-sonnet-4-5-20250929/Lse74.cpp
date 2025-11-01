
#include <iostream>
#include <cstdlib>

typedef struct {
    int status;
} Person;

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        person->status = 0;
    }
    return person;
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        std::cout << "Test 1 - Status: " << p1->status << std::endl;
        free(p1);
    }
    
    // Test case 2
    Person* p2 = allocatePerson();
    if (p2 != NULL) {
        std::cout << "Test 2 - Status: " << p2->status << std::endl;
        free(p2);
    }
    
    // Test case 3
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        std::cout << "Test 3 - Status: " << p3->status << std::endl;
        free(p3);
    }
    
    // Test case 4
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        std::cout << "Test 4 - Status: " << p4->status << std::endl;
        free(p4);
    }
    
    // Test case 5
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        std::cout << "Test 5 - Status: " << p5->status << std::endl;
        free(p5);
    }
    
    return 0;
}
