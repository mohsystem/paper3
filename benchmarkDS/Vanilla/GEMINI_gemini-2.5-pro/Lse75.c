#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int id;
    int status;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person) using malloc,
 * sets its status to 0, and returns a pointer to the beginning of the block.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* allocatePerson(int id) {
    // Allocate a block of memory on the heap
    Person* p = (Person*)malloc(sizeof(Person));

    // Check if malloc was successful
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Initialize the person's data
    p->id = id;
    p->status = 0;

    return p;
}

int main() {
    printf("C Test Cases:\n");
    Person* people[5];

    // 5 Test cases
    for (int i = 0; i < 5; ++i) {
        people[i] = allocatePerson(i + 1);
        if (people[i] != NULL) {
            printf("Test Case %d: Allocated Person at %p with id=%d, status=%d\n",
                   i + 1, (void*)people[i], people[i]->id, people[i]->status);
        } else {
             printf("Test Case %d: Allocation failed.\n", i + 1);
        }
    }

    // Clean up the dynamically allocated memory to prevent memory leaks
    printf("\nCleaning up memory...\n");
    for (int i = 0; i < 5; ++i) {
        if (people[i] != NULL) {
            free(people[i]);
        }
    }

    return 0;
}