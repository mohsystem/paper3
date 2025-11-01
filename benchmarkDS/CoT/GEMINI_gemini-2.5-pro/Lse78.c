#include <stdio.h>
#include <stdlib.h> // Required for malloc() and free()

// Define the Person structure
struct Person {
    int status;
    // Other attributes can be added here
};

/**
 * Allocates a block of memory for a Person on the heap,
 * sets its status to 0, and returns a pointer to it.
 *
 * @return A pointer to the newly allocated Person, or NULL if allocation fails.
 */
struct Person* allocatePerson() {
    // Allocate memory for a single Person struct.
    struct Person* p = (struct Person*) malloc(sizeof(struct Person));

    // Security check: Always verify that malloc() returned a valid pointer.
    if (p == NULL) {
        // malloc failed to allocate memory; return NULL to indicate failure.
        return NULL;
    }

    // Initialize the data in the allocated block.
    p->status = 0;
    
    return p;
}

int main() {
    printf("--- C Test Cases ---\n");
    
    // Run 5 test cases
    for (int i = 1; i <= 5; ++i) {
        printf("Test Case %d:\n", i);
        struct Person* p = allocatePerson();

        // Security check: Always check the returned pointer before using it.
        if (p != NULL) {
            printf("Person allocated successfully at address: %p\n", (void*)p);
            printf("Person status: %d\n", p->status);

            // Security best practice: Free the allocated memory to prevent memory leaks.
            free(p);
            printf("Memory freed.\n");
        } else {
            printf("Failed to allocate Person.\n");
        }
        printf("\n");
    }
    
    return 0;
}