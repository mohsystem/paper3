#include <stdio.h>
#include <stdlib.h>

// Define the Person structure
typedef struct {
    int status;
    int id;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person), initializes its status
 * field to 0, and returns a pointer to the beginning of the block.
 *
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* createPerson() {
    // Allocate a block of memory for one Person struct.
    // Use calloc to allocate and zero-initialize the memory, which is a
    // secure way to ensure all fields, including status, start at 0.
    // Alternatively, one could use malloc and manually set the status.
    Person* newPerson = (Person*)malloc(sizeof(Person));

    // Security: Check if malloc returned NULL (which happens on failure).
    if (newPerson == NULL) {
        perror("Failed to allocate memory for Person");
        return NULL; // Return NULL to signal failure.
    }

    // Explicitly set the status field to 0 as per the requirement.
    newPerson->status = 0;
    newPerson->id = -1; // Initialize other fields to a default value.
    
    return newPerson;
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    Person* people[5] = {NULL}; // Array to hold pointers for cleanup

    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\n", i + 1);
        people[i] = createPerson();

        // Security: Always check for a NULL pointer before dereferencing it.
        if (people[i] != NULL) {
            printf("  Person created successfully.\n");
            printf("  Initial status: %d\n", people[i]->status);
        } else {
            printf("  Failed to create person.\n");
        }
        printf("--------------------\n");
    }

    // Security: Free all allocated memory to prevent memory leaks.
    printf("Cleaning up allocated memory...\n");
    for (int i = 0; i < 5; i++) {
        if (people[i] != NULL) {
            free(people[i]);
        }
    }
    printf("Cleanup complete.\n");

    return 0;
}