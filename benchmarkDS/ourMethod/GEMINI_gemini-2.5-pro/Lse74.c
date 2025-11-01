#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
    int id;
} person;

/**
 * Allocates a block of memory of size sizeof(person), initializes it,
 * and sets the status field to 0.
 *
 * @param id The identifier for the new person.
 * @return A pointer to the beginning of the allocated block, or NULL on failure.
 */
person* allocatePerson(int id) {
    // Allocate a block of memory. sizeof() is used to ensure the correct size.
    // calloc is often safer as it zero-initializes memory, but prompt implies
    // manual initialization, so malloc is also appropriate.
    person* p = (person*)malloc(sizeof(person));

    // ALWAYS check the return value of malloc.
    if (p == NULL) {
        perror("Memory allocation failed for person");
        return NULL;
    }

    // Set the status field to 0 as required.
    p->status = 0;
    p->id = id;

    return p;
}

void run_test(int test_num, int id) {
    person* p = allocatePerson(id);
    if (p != NULL && p->status == 0) {
        printf("Test Case %d PASSED: person{id=%d, status=%d}\n", test_num, p->id, p->status);
    } else {
        fprintf(stderr, "Test Case %d FAILED: Allocation failed or status not 0.\n", test_num);
    }
    // Free the allocated memory to prevent memory leaks.
    free(p);
}

int main() {
    printf("Running C test cases...\n");
    
    // Test cases
    run_test(1, 401);
    run_test(2, 402);
    run_test(3, 403);
    run_test(4, 404);
    run_test(5, 405);
    
    return 0;
}