
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
} Person;

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    
    if (person == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    // Initialize all fields to prevent undefined behavior
    memset(person, 0, sizeof(Person));
    person->status = 0;
    
    return person;
}

void deallocatePerson(Person* person) {
    if (person != NULL) {
        free(person);
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Test 1 - Status: %d\\n", p1->status);
    }
    
    // Test case 2: Multiple allocations
    Person* p2 = allocatePerson();
    if (p2 != NULL) {
        printf("Test 2 - Status: %d\\n", p2->status);
    }
    
    // Test case 3: Verify independence
    if (p1 != NULL) {
        p1->status = 5;
    }
    if (p2 != NULL) {
        printf("Test 3 - P2 Status (should be 0): %d\\n", p2->status);
    }
    
    // Test case 4: Another allocation
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        printf("Test 4 - Status: %d\\n", p3->status);
    }
    
    // Test case 5: Verify initialization
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        printf("Test 5 - Status: %d\\n", p4->status);
    }
    
    // Clean up to prevent memory leaks
    deallocatePerson(p1);
    deallocatePerson(p2);
    deallocatePerson(p3);
    deallocatePerson(p4);
    
    return 0;
}
