#include <stdio.h>
#include <stdlib.h>

// A simple struct to represent a person
struct person {
    int age;
    float height;
};

/**
 * Allocates a block of memory of size sizeof(struct person).
 * @return A pointer to the beginning of the block, or NULL on failure.
 */
struct person* allocatePerson() {
    // 1. Allocate a block of memory using malloc
    struct person* ptr = (struct person*)malloc(sizeof(struct person));

    // 2. Security Check: malloc returns NULL if it fails to allocate memory.
    // The caller must handle this case.
    if (ptr == NULL) {
        // Optionally, print an error message here, but returning NULL is key.
        // fprintf(stderr, "Error: Memory allocation failed in allocatePerson.\n");
    }

    return ptr;
}

int main() {
    printf("--- C Test Cases ---\n");

    for (int i = 1; i <= 5; i++) {
        printf("Test Case %d:\n", i);

        // 1. Allocate memory for a person struct
        struct person* p = allocatePerson();

        // 2. Security Check: Always verify that the returned pointer is not NULL
        // before dereferencing it to avoid undefined behavior (e.g., a crash).
        if (p != NULL) {
            // 3. Initialize the struct's members
            p->age = 20 + i;
            p->height = 170.5f + i;

            // 4. Print the struct's state
            printf("  Allocated and initialized: Person{age=%d, height=%.1f}\n", p->age, p->height);

            // 5. Deallocate the memory using free() to prevent memory leaks.
            // Every malloc() must have a corresponding free().
            free(p);
            p = NULL; // Good practice to set pointer to NULL after freeing to prevent use-after-free bugs.
            printf("  Memory deallocated.\n");
        } else {
            printf("  Memory allocation failed.\n");
        }
        printf("\n");
    }

    return 0;
}