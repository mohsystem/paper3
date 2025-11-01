#include <stdio.h>
#include <stdlib.h>

// Define the Person structure
typedef struct {
    int status;
} Person;

/**
 * Allocates a block of memory for a Person, initializes status to 0,
 * and returns a pointer to it.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // Allocate memory of size sizeof(Person).
    Person* p = (Person*)malloc(sizeof(Person));

    // Always check if malloc was successful.
    if (p == NULL) {
        // On failure, malloc returns NULL.
        return NULL;
    }

    // Initialize the allocated memory.
    p->status = 0;

    return p;
}

int main() {
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d\n", i + 1);
        Person* p = allocatePerson();

        // Always check if the pointer is valid (not NULL) before dereferencing it.
        if (p != NULL) {
            printf("Person allocated successfully at address: %p\n", (void*)p);
            printf("Person status: %d\n", p->status);
            
            // Free the allocated memory to prevent memory leaks.
            free(p);
            // Good practice to set pointer to NULL to avoid dangling pointers (use-after-free).
            p = NULL; 
            printf("Memory freed.\n");
        } else {
            // Use fprintf to print errors to stderr.
            fprintf(stderr, "Person allocation failed.\n");
        }
        printf("--------------------\n");
    }
    return 0;
}