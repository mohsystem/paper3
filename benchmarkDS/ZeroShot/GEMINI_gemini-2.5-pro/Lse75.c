#include <stdio.h>
#include <stdlib.h> // For malloc, calloc, free

// A struct to represent a person.
typedef struct {
    int status;
    int id;
} Person;

/**
 * Allocates memory for a new person and initializes it.
 * The function allocates a block of memory of size sizeof(Person)
 * and returns a pointer to the beginning of the block.
 *
 * @param id The identifier for the new person.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* createPerson(int id) {
    // Use calloc for secure allocation. It provides two benefits:
    // 1. It initializes the allocated memory to zero, which sets 'status' to 0.
    // 2. It helps prevent integer overflow by taking number of elements and size
    //    as separate arguments.
    Person* newPerson = (Person*)calloc(1, sizeof(Person));

    // Security check: Always check the return value of allocation functions.
    if (newPerson == NULL) {
        // On failure, calloc returns NULL.
        perror("Error: Unable to allocate memory for new person");
        return NULL;
    }

    // Although calloc zeroed the memory (so status is 0), we set the id.
    newPerson->id = id;
    // newPerson->status is already 0 due to calloc.

    return newPerson;
}

int main() {
    printf("C Test Cases:\n");
    Person* people[5] = {NULL};
    int created_count = 0;

    // 5 Test Cases
    for (int i = 0; i < 5; ++i) {
        people[i] = createPerson((i + 1) * 10);

        if (people[i] != NULL) {
            printf("Test Case %d: Created Person{id=%d, status=%d}\n",
                   i + 1, people[i]->id, people[i]->status);
            created_count++;
        } else {
            printf("Test Case %d: Person creation failed.\n", i + 1);
        }
    }

    // Security: It is crucial to free all dynamically allocated memory
    // to prevent memory leaks.
    printf("\nFreeing memory...\n");
    for (int i = 0; i < 5; ++i) {
        if (people[i] != NULL) {
            free(people[i]);
            people[i] = NULL; // Good practice to avoid dangling pointers.
        }
    }
    
    printf("Total people created: %d\n", created_count);

    return 0;
}