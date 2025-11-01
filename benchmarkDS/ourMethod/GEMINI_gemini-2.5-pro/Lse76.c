#include <stdio.h>
#include <stdlib.h>

/**
 * A simple structure to represent a person.
 * Using typedef for convenience.
 */
typedef struct {
    int status;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person) and returns a pointer
 * to the starting of the block. The new person's status is set to 0.
 *
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // Allocate memory for one Person struct.
    Person* p = (Person*)malloc(sizeof(Person));

    // ALWAYS check if malloc returned NULL.
    if (p == NULL) {
        // Return NULL to the caller to indicate failure.
        return NULL;
    }

    // If allocation was successful, initialize the memory.
    p->status = 0;

    return p;
}

int main() {
    printf("--- Running C Test Cases ---\n");
    
    const int NUM_TESTS = 5;
    Person* allocated_persons[NUM_TESTS] = {NULL};
    int allocated_count = 0;

    // Test cases: Allocate 5 Person objects
    for (int i = 0; i < NUM_TESTS; ++i) {
        printf("Test Case #%d\n", i + 1);
        Person* p = allocatePerson();

        if (p != NULL) {
            printf("  Allocation successful at address: %p\n", (void*)p);
            printf("  Status is correctly set to: %d\n", p->status);
            allocated_persons[allocated_count++] = p;
        } else {
            fprintf(stderr, "  Memory allocation failed.\n");
            // Clean up any memory that was successfully allocated before the failure.
            for (int j = 0; j < allocated_count; ++j) {
                free(allocated_persons[j]);
            }
            return 1; // Exit with an error code
        }
        printf("\n");
    }

    // IMPORTANT: Free all dynamically allocated memory to prevent memory leaks.
    printf("Cleaning up allocated memory...\n");
    for (int i = 0; i < allocated_count; ++i) {
        free(allocated_persons[i]);
    }
    printf("Cleanup complete.\n");

    return 0;
}