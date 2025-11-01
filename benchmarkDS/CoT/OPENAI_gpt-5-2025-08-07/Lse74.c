#include <stdio.h>
#include <stdlib.h>

// Step 1: Define struct person with a status field.
typedef struct person {
    int status;
} person;

// Step 3: Secure allocation using malloc(sizeof(person)), returning pointer to the block.
// Sets the status field to 0 explicitly.
person* allocate_person(void) {
    // Step 2: Security - check allocation result.
    person* p = (person*)malloc(sizeof(person));
    if (p == NULL) {
        return NULL;
    }
    // Step 3: Initialize fields securely.
    p->status = 0;
    return p;
}

// Step 5: Main with 5 test cases.
int main(void) {
    person* p1 = allocate_person();
    person* p2 = allocate_person();
    person* p3 = allocate_person();
    person* p4 = allocate_person();
    person* p5 = allocate_person();

    if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL) {
        fprintf(stderr, "Allocation failed in one of the test cases.\n");
        free(p1);
        free(p2);
        free(p3);
        free(p4);
        free(p5);
        return 1;
    }

    printf("Test1 status: %d\n", p1->status);
    printf("Test2 status: %d\n", p2->status);
    printf("Test3 status: %d\n", p3->status);
    printf("Test4 status: %d\n", p4->status);
    printf("Test5 status: %d\n", p5->status);

    // Cleanup to prevent memory leaks.
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);

    return 0;
}