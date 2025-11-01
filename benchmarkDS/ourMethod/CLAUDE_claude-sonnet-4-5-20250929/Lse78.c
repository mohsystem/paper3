
// C: Manual memory management with careful allocation checking and cleanup
// Each allocation is verified and freed exactly once to prevent leaks

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Person structure with status field
typedef struct {
    int status;
} Person;

// Allocates a new Person with status initialized to 0
// Returns NULL on allocation failure (fail-safe behavior)
Person* allocatePerson(void) {
    // Use calloc to zero-initialize the structure (status becomes 0)
    // calloc is safer than malloc as it initializes memory to zero
    Person* newPerson = (Person*)calloc(1, sizeof(Person));
    
    // Check allocation result - critical for security and stability
    if (newPerson == NULL) {
        // Allocation failed - return NULL to indicate error
        // Caller must check return value before use
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Status is already 0 from calloc, but explicit assignment for clarity
    newPerson->status = 0;
    
    return newPerson;
}

// Helper function to free a Person and set pointer to NULL
void freePerson(Person** person) {
    if (person != NULL && *person != NULL) {
        // Free the memory
        free(*person);
        // Set pointer to NULL to prevent use-after-free
        *person = NULL;
    }
}

int main(void) {
    // Test case 1: Basic allocation
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Test 1: Person{status=%d}\\n", p1->status);
        freePerson(&p1);
    }
    
    // Test case 2: Multiple allocations
    Person* p2 = allocatePerson();
    Person* p3 = allocatePerson();
    if (p2 != NULL && p3 != NULL) {
        printf("Test 2: Person{status=%d}, Person{status=%d}\\n", 
               p2->status, p3->status);
    }
    freePerson(&p2);
    freePerson(&p3);
    
    // Test case 3: Verify initial status
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        printf("Test 3: Status is %d\\n", p4->status);
        freePerson(&p4);
    }
    
    // Test case 4: Modify status after allocation
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        p5->status = 1;
        printf("Test 4: Modified status to %d\\n", p5->status);
        freePerson(&p5);
    }
    
    // Test case 5: Array of person pointers
    #define ARRAY_SIZE 3
    Person* people[ARRAY_SIZE] = {NULL};  // Initialize all to NULL
    int allocated = 0;
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        people[i] = allocatePerson();
        if (people[i] != NULL) {
            allocated++;
        } else {
            // Allocation failed - clean up and exit
            for (int j = 0; j < i; j++) {
                freePerson(&people[j]);
            }
            fprintf(stderr, "Test 5: Allocation failed\\n");
            return 1;
        }
    }
    
    printf("Test 5: Allocated %d persons\\n", allocated);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        printf("  Person %d: Person{status=%d}\\n", i, people[i]->status);
    }
    
    // Clean up all allocated persons
    for (int i = 0; i < ARRAY_SIZE; i++) {
        freePerson(&people[i]);
    }
    
    return 0;
}
