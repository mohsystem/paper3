/*
Chain-of-Through Process:
1) Problem understanding: Implement a function that allocates sizeof(Person) and returns a pointer to the block, with status set to 0.
2) Security requirements: Check malloc result; avoid use-after-free; clean up resources.
3) Secure coding generation: Use malloc with sizeof, validate pointer, initialize memory, and free in tests.
4) Code review: Ensure no buffer overflows, null dereference checks, and proper frees.
5) Secure code output: Final code with 5 test cases demonstrating safe allocation and cleanup.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
} Person;

// Secure allocation: allocates a block of memory of size sizeof(Person) and returns pointer to its beginning.
// Returns NULL on failure.
Person* allocate_person(void) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        return NULL;
    }
    // Zero-initialize to ensure all fields are predictable
    memset(p, 0, sizeof(Person));
    // Explicitly set status to 0 (redundant due to memset but explicit for clarity)
    p->status = 0;
    return p;
}

int main(void) {
    Person* persons[5] = {0};
    for (int i = 0; i < 5; ++i) {
        persons[i] = allocate_person();
        if (persons[i] == NULL) {
            printf("Test %d: Allocation failed (NULL).\n", i + 1);
        } else {
            printf("Test %d: Person allocated, status = %d\n", i + 1, persons[i]->status);
        }
    }

    // Cleanup
    for (int i = 0; i < 5; ++i) {
        free(persons[i]);
        persons[i] = NULL;
    }

    return 0;
}