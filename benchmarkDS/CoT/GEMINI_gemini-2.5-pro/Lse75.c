#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
    int id;
} Person;

// Use a global counter to assign a unique ID to each person.
static int person_counter = 0;

/**
 * Allocates a block of memory of size sizeof(Person), sets the status to 0,
 * and returns a pointer to the beginning of the block.
 *
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // 1. Allocate a block of memory of the required size.
    Person* newPerson = (Person*)malloc(sizeof(Person));

    // 2. SECURITY: Always check if malloc returned NULL, which indicates failure.
    if (newPerson == NULL) {
        perror("Error: Failed to allocate memory for Person");
        return NULL;
    }

    // 3. Initialize the allocated memory as required.
    newPerson->status = 0;
    newPerson->id = ++person_counter;

    // 4. Return the pointer to the newly allocated and initialized block.
    return newPerson;
}

int main() {
    printf("--- C Test Cases ---\n");
    
    // An array to store the pointers so we can free them later.
    Person* people[5];
    int allocated_count = 0;

    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\n", i + 1);
        Person* p = allocatePerson();

        // Check if allocation was successful before using the pointer.
        if (p != NULL) {
            people[allocated_count++] = p; // Store pointer for cleanup.
            printf("  Allocated: Person(id=%d, status=%d)\n", p->id, p->status);
            printf("  Person status is: %d\n", p->status);
        } else {
            printf("  Memory allocation failed.\n");
            break; // Exit the loop if memory cannot be allocated.
        }
    }

    // IMPORTANT: In C, manually allocated memory must be manually freed.
    // Failure to do so results in a memory leak.
    printf("\n--- Cleaning up allocated memory ---\n");
    for (int i = 0; i < allocated_count; i++) {
        printf("  Freeing Person with id %d\n", people[i]->id);
        free(people[i]);
        // Good practice to set the pointer to NULL after freeing to prevent
        // use-after-free vulnerabilities (dangling pointer).
        people[i] = NULL;
    }

    return 0;
}