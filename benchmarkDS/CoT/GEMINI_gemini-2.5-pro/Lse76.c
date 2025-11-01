#include <stdio.h>
#include <stdlib.h>

// Represents the person structure
typedef struct {
    int status;
} person;

/**
 * Allocates a block of memory of size sizeof(person) using malloc.
 * If allocation is successful, it sets the person's status to 0.
 *
 * @return A pointer to the starting of the block, or NULL if allocation fails.
 */
person* allocatePerson() {
    // Allocate a block of memory of the size of one person struct.
    person* p = (person*)malloc(sizeof(person));

    // Security: Check if malloc returned NULL, which indicates allocation failure.
    if (p == NULL) {
        // Return NULL to the caller to signal failure.
        return NULL;
    }

    // If allocation was successful, set the status to 0.
    p->status = 0;

    // Return the pointer to the newly allocated and initialized block.
    return p;
}

int main() {
    printf("Running 5 test cases in C:\n");
    for (int i = 0; i < 5; i++) {
        printf("--- Test Case %d ---\n", i + 1);
        person* p = allocatePerson();

        // Security: Always check the returned pointer before dereferencing it.
        if (p != NULL) {
            printf("Person allocated successfully.\n");
            printf("Person status: %d\n", p->status);
            
            // Security: Free the dynamically allocated memory to prevent memory leaks.
            free(p);
            p = NULL; // Best practice: set pointer to NULL after freeing.
        } else {
            // Handle the memory allocation error.
            fprintf(stderr, "Failed to allocate memory for person.\n");
        }
    }
    return 0;
}