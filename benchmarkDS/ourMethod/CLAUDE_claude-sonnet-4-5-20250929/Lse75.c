
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
} person;

/* Allocates a new person with status initialized to 0
 * Returns NULL on allocation failure
 * Caller is responsible for freeing the returned pointer
 */
person* allocate_person(void) {
    /* Allocate memory for person struct - checking for NULL is critical */
    person* p = (person*)malloc(sizeof(person));
    
    /* Check if allocation succeeded */
    if (p == NULL) {
        return NULL;
    }
    
    /* Initialize status to 0 - explicit initialization prevents uninitialized memory */
    p->status = 0;
    
    return p;
}

/* Safely deallocate person and clear sensitive data if any */
void free_person(person* p) {
    if (p != NULL) {
        /* Clear memory before freeing to prevent information leakage */
        memset(p, 0, sizeof(person));
        free(p);
    }
}

int main(void) {
    /* Test case 1: Normal allocation */
    person* p1 = allocate_person();
    if (p1 != NULL) {
        printf("Test 1 - Person allocated, status: %d\\n", p1->status);
        free_person(p1);
    } else {
        printf("Test 1 - Allocation failed\\n");
    }
    
    /* Test case 2: Verify status is 0 */
    person* p2 = allocate_person();
    if (p2 != NULL) {
        printf("Test 2 - Status is zero: %d\\n", (p2->status == 0));
        free_person(p2);
    }
    
    /* Test case 3: Multiple allocations */
    person* p3 = allocate_person();
    if (p3 != NULL) {
        printf("Test 3 - Multiple allocations work, status: %d\\n", p3->status);
        free_person(p3);
    }
    
    /* Test case 4: Verify independence of objects */
    person* p4 = allocate_person();
    person* p5 = allocate_person();
    if (p4 != NULL && p5 != NULL) {
        p4->status = 1;
        printf("Test 4 - Objects are independent: %d\\n", (p5->status == 0));
        free_person(p4);
        free_person(p5);
    }
    
    /* Test case 5: Null check handling */
    person* p6 = allocate_person();
    printf("Test 5 - Null check passed: %d\\n", (p6 != NULL));
    free_person(p6);
    
    return 0;
}
