
#include <iostream>
#include <cstring>
#include <cstdlib>

struct Person {
    int status;
    char name[50];
};

Person* createPerson() {
    // Allocate memory for Person structure
    Person* person = (Person*)malloc(sizeof(Person));
    
    // Check if allocation was successful
    if (person == NULL) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return NULL;
    }
    
    // Initialize fields
    person->status = 0;
    
    // Safe string copy with bounds checking
    const char* sourceName = "John";
    size_t nameLen = strlen(sourceName);
    if (nameLen >= sizeof(person->name)) {
        nameLen = sizeof(person->name) - 1;
    }
    strncpy(person->name, sourceName, nameLen);
    person->name[nameLen] = '\\0'; // Ensure null termination
    
    return person;
}

void printPerson(Person* person) {
    if (person != NULL) {
        std::cout << "Person{status=" << person->status 
                  << ", name='" << person->name << "'}" << std::endl;
    }
}

int main() {
    // Test case 1: Basic creation
    Person* person1 = createPerson();
    if (person1 != NULL) {
        std::cout << "Test 1: ";
        printPerson(person1);
        free(person1);
    }
    
    // Test case 2: Verify status
    Person* person2 = createPerson();
    if (person2 != NULL) {
        std::cout << "Test 2 - Status: " << person2->status << std::endl;
        free(person2);
    }
    
    // Test case 3: Verify name
    Person* person3 = createPerson();
    if (person3 != NULL) {
        std::cout << "Test 3 - Name: " << person3->name << std::endl;
        free(person3);
    }
    
    // Test case 4: Multiple instances
    Person* person4 = createPerson();
    Person* person5 = createPerson();
    if (person4 != NULL && person5 != NULL) {
        std::cout << "Test 4 - Multiple: ";
        printPerson(person4);
        std::cout << "                   ";
        printPerson(person5);
        free(person4);
        free(person5);
    }
    
    // Test case 5: Memory management
    Person* person6 = createPerson();
    if (person6 != NULL) {
        std::cout << "Test 5 - Memory managed: ";
        printPerson(person6);
        free(person6);
    }
    
    return 0;
}
