#include <stdio.h>
#include <stdlib.h> // Required for calloc and free

typedef struct {
    // Members can be added here, e.g., char name[50]; int age;
    int placeholder; // To ensure the struct is not empty.
} person;

/**
 * Securely allocates a block of memory for a person struct.
 * Uses calloc instead of malloc. calloc is generally more secure because:
 * 1. It helps prevent integer overflow when calculating size by taking
 *    number of items and size as separate arguments.
 * 2. It initializes the allocated memory to zero, preventing the use of
 *    uninitialized data (which could be an information disclosure vulnerability).
 *
 * @return A pointer to the newly allocated person struct, or NULL on failure.
 */
person* allocatePerson() {
    // Allocate memory for one person struct. The memory is zero-initialized.
    person* p = (person*)calloc(1, sizeof(person));
    return p;
}

/**
 * Securely deallocates memory for a person struct.
 *
 * @param p A pointer to the person struct to be deallocated.
 */
void deallocatePerson(person* p) {
    // `free` on a NULL pointer is a safe no-op, so an explicit check is not
    // strictly necessary, but it is often considered good practice.
    free(p);
}

int main() {
    printf("Running C Test Cases...\n");

    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);

        // 1. Allocate memory for a person struct.
        person* p = allocatePerson();

        // 2. Securely check if the allocation was successful.
        if (p != NULL) {
            printf("Successfully allocated a person struct at address: %p\n", (void*)p);
            // The struct can now be used.
        } else {
            // Use stderr for error messages.
            fprintf(stderr, "Failed to allocate memory for person struct.\n");
        }

        // 3. Deallocate the memory to prevent memory leaks.
        deallocatePerson(p);
        if (p != NULL) {
            printf("Deallocated memory for person struct.\n");
        }
        p = NULL; // Good practice to nullify pointer after deallocation.
    }

    return 0;
}