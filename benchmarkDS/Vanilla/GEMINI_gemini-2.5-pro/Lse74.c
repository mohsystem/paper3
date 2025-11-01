#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person) and returns a pointer
 * to the beginning of the block. The new person's status field is set to 0.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson() {
    // Allocate a block of memory of size sizeof(Person)
    Person* p = (Person*)malloc(sizeof(Person));

    // Check if malloc was successful
    if (p != NULL) {
        // Set the status field to 0
        p->status = 0;
    }

    return p;
}

int main() {
    printf("--- C Test Cases ---\n");
    Person* people[5]; // Array to hold pointers for later free-ing

    for (int i = 0; i < 5; ++i) {
        people[i] = allocatePerson();
        printf("Test Case %d:\n", i + 1);
        if (people[i] != NULL) {
            printf("  Person allocated at: %p\n", (void*)people[i]);
            printf("  Person status: %d\n", people[i]->status);
        } else {
            printf("  Allocation failed.\n");
        }
    }

    // Clean up allocated memory to prevent memory leaks
    printf("\nCleaning up memory...\n");
    for (int i = 0; i < 5; ++i) {
        if(people[i] != NULL) {
            free(people[i]);
        }
    }
    printf("Cleanup complete.\n");

    return 0;
}