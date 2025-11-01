#include <stdio.h>
#include <stdlib.h> // Required for malloc, calloc, free

// Define the person structure
typedef struct {
    int status;
    int id;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person).
 * Returns a pointer to the beginning of the block.
 * This function uses calloc for secure allocation, which initializes memory to zero.
 * @param id The identifier for the new person.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* createPerson(int id) {
    // Use calloc to allocate and zero-initialize memory. This is generally safer
    // than malloc + memset, as it's an atomic operation.
    // It allocates memory for 1 element of size sizeof(Person).
    // Zero-initialization ensures 'status' is 0.
    Person* newPerson = (Person*)calloc(1, sizeof(Person));

    // CRITICAL: Always check the return value of memory allocation functions.
    if (newPerson == NULL) {
        // A null return indicates that the allocation failed.
        perror("Error: Failed to allocate memory for Person");
        return NULL;
    }

    // Although calloc already set status to 0, we can set other fields.
    newPerson->id = id;

    return newPerson;
}

int main() {
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\n", i + 1);
        Person* p = createPerson(i);

        // Check if the allocation was successful before using the pointer.
        if (p != NULL) {
            printf("  Successfully created Person with ID: %d\n", p->id);
            printf("  Person status field is: %d\n", p->status);

            // CRITICAL: Free the allocated memory to prevent memory leaks.
            // For every successful 'malloc' or 'calloc', there must be a corresponding 'free'.
            free(p);
        } else {
            printf("  Failed to create Person.\n");
            // If allocation fails, exit the loop as subsequent attempts will likely fail too.
            break;
        }
    }
    return 0;
}