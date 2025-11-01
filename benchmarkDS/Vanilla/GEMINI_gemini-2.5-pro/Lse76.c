#include <stdio.h>
#include <stdlib.h>

// Define the Person structure
typedef struct {
    int status;
    int age;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person), sets its status to 0,
 * and returns a pointer to the starting of the block.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // 1. Allocate a block of memory of size sizeof(Person).
    Person* newPerson = (Person*)malloc(sizeof(Person));

    // 2. Check if malloc returned a valid pointer.
    if (newPerson != NULL) {
        // 3. Set the status of the new person to 0.
        newPerson->status = 0;
        newPerson->age = -1; // Good practice to initialize all members.
    }

    // 4. Return the pointer.
    return newPerson;
}

int main() {
    printf("--- C Test Cases ---\n");
    for (int i = 1; i <= 5; ++i) {
        Person* p = allocatePerson();
        if (p != NULL) {
            printf("Test Case %d: Allocated Person at %p. Status: %d\n", i, (void*)p, p->status);
            // Deallocate the memory to prevent memory leaks.
            free(p);
        } else {
            printf("Test Case %d: Memory allocation failed.\n", i);
        }
    }
    return 0;
}