
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
    // Test case 1: Basic allocation
    Person* person1 = allocatePerson();
    std::cout << "Test 1 - Status: " << person1->status << std::endl;
    
    // Test case 2: Multiple allocations
    Person* person2 = allocatePerson();
    std::cout << "Test 2 - Status: " << person2->status << std::endl;
    
    // Test case 3: Verify independence
    person1->status = 1;
    Person* person3 = allocatePerson();
    std::cout << "Test 3 - Status: " << person3->status << std::endl;
    
    // Test case 4: Check not null
    Person* person4 = allocatePerson();
    std::cout << "Test 4 - Not null: " << (person4 != NULL) << std::endl;
    
    // Test case 5: Verify initial values
    Person* person5 = allocatePerson();
    std::cout << "Test 5 - Status: " << person5->status << ", Age: " << person5->age << std::endl;
    
    // Free allocated memory
    free(person1);
    free(person2);
    free(person3);
    free(person4);
    free(person5);
    
    return 0;
}
