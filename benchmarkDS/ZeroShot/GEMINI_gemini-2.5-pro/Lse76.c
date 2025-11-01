#include <stdio.h>
#include <stdlib.h> // Required for malloc, calloc, free

// Define the Person structure
typedef struct {
    int status;
} Person;

/**
 * Allocates a block of memory for a Person object and initializes it.
 * Using calloc is a secure practice because:
 * 1. It checks for integer overflow when calculating size (count * size).
 * 2. It zero-initializes the allocated memory, preventing the use of
 *    uninitialized data. This automatically sets 'status' to 0.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // calloc(number_of_elements, size_of_each_element)
    Person* p = (Person*)calloc(1, sizeof(Person));
    return p;
}

int main() {
    printf("Running C test cases...\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        Person* p = allocatePerson();

        // Security: Always check if the allocation was successful
        if (p != NULL) {
            printf("  Person allocated successfully.\n");
            printf("  Person status: %d\n", p->status);

            // Security: Free the allocated memory to prevent memory leaks
            free(p);
            p = NULL; // Best practice to avoid dangling pointers
        } else {
            fprintf(stderr, "  Person allocation failed.\n");
        }
    }
    return 0;
}