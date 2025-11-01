
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char name[50];
} Person;

Person* createPerson() {
    // Allocate memory for Person structure
    Person* person = (Person*)malloc(sizeof(Person));
    
    // Check if allocation was successful
    if (person == NULL) {
        fprintf(stderr, "Memory allocation failed!\\n");
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
        printf("Person{status=%d, name='%s'}\\n", person->status, person->name);
    }
}

int main() {
    // Test case 1: Basic creation
    Person* person1 = createPerson();
    if (person1 != NULL) {
        printf("Test 1: ");
        printPerson(person1);
        free(person1);
    }
    
    // Test case 2: Verify status
    Person* person2 = createPerson();
    if (person2 != NULL) {
        printf("Test 2 - Status: %d\\n", person2->status);
        free(person2);
    }
    
    // Test case 3: Verify name
    Person* person3 = createPerson();
    if (person3 != NULL) {
        printf("Test 3 - Name: %s\\n", person3->name);
        free(person3);
    }
    
    // Test case 4: Multiple instances
    Person* person4 = createPerson();
    Person* person5 = createPerson();
    if (person4 != NULL && person5 != NULL) {
        printf("Test 4 - Multiple: ");
        printPerson(person4);
        printf("                   ");
        printPerson(person5);
        free(person4);
        free(person5);
    }
    
    // Test case 5: Memory management
    Person* person6 = createPerson();
    if (person6 != NULL) {
        printf("Test 5 - Memory managed: ");
        printPerson(person6);
        free(person6);
    }
    
    return 0;
}
