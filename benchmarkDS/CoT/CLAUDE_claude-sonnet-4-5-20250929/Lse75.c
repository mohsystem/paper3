
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
    
    memset(person, 0, sizeof(Person));
    person->status = 0;
    
    return person;
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
    
    // Test case 3: Verify independent objects
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        p3->status = 5;
        printf("Test 3 - Modified status: %d\\n", p3->status);
    }
    
    // Test case 4: Check original unchanged
    if (p1 != NULL) {
        printf("Test 4 - Original status: %d\\n", p1->status);
    }
    
    // Test case 5: Multiple allocations in loop
    printf("Test 5 - Allocating 5 persons:\\n");
    Person* persons[5];
    for (int i = 0; i < 5; i++) {
        persons[i] = allocatePerson();
        if (persons[i] != NULL) {
            printf("  Person %d status: %d\\n", i+1, persons[i]->status);
        }
    }
    
    // Clean up memory
    free(p1);
    free(p2);
    free(p3);
    for (int i = 0; i < 5; i++) {
        if (persons[i] != NULL) {
            free(persons[i]);
        }
    }
    
    return 0;
}
