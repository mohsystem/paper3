
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Person structure with status field
typedef struct {
    int status;
} Person;

// Memory allocation method that allocates a new Person and sets status to 0
// Returns pointer to allocated Person or NULL on failure
Person* allocatePerson(void) {
    // Allocate memory for Person structure
    Person* newPerson = (Person*)malloc(sizeof(Person));
    
    // Check if allocation was successful (critical for C)
    if (newPerson == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Person\\n");
        return NULL;
    }
    
    // Initialize the entire structure to zero (secure initialization)
    // This ensures all fields are in a known state
    memset(newPerson, 0, sizeof(Person));
    
    // Explicitly set status to 0 (though memset already did this)
    newPerson->status = 0;
    
    return newPerson;
}

// Helper function to safely deallocate Person
void freePerson(Person* person) {
    if (person != NULL) {
        // Clear sensitive data before freeing (defensive practice)
        memset(person, 0, sizeof(Person));
        free(person);
    }
}

int main(void) {
    // Test case 1: Basic allocation
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
        Person* p3 = allocatePerson();
        if (p3 != NULL) {
            printf("Test 3 - p1 Status: %d, p3 Status: %d\\n", 
                   p1->status, p3->status);
            freePerson(p3);
        }
    }
    
    // Test case 4: Array of persons
    Person* persons[3] = {NULL, NULL, NULL};
    for (int i = 0; i < 3; i++) {
        persons[i] = allocatePerson();
        if (persons[i] != NULL) {
            printf("Test 4.%d - Status: %d\\n", i+1, persons[i]->status);
        }
    }
    
    // Test case 5: Null safety check
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        printf("Test 5 - Allocation successful, Status: %d\\n", p5->status);
    }
    
    // Clean up all allocated memory (prevent memory leaks)
    freePerson(p1);
    freePerson(p2);
    for (int i = 0; i < 3; i++) {
        freePerson(persons[i]);
    }
    freePerson(p5);
    
    return 0;
}
