
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define person structure */
typedef struct {
    int status;
    char name[64];
    int age;
} person;

/* Secure memory allocation function for person structure
 * Returns: pointer to allocated person on success, NULL on failure
 * Security measures:
 * - Check allocation result for NULL (Rules#8)
 * - Initialize all memory to zero to avoid uninitialized data exposure (Rules#8)
 * - Set status explicitly after validation (Rules#8)
 */
person* allocate_person(void) {
    /* Allocate memory for person structure
     * Use calloc instead of malloc to zero-initialize all fields (Rules#8)
     * This prevents information leakage from uninitialized memory
     */
    person *p = (person*)calloc(1, sizeof(person));
    
    /* Check allocation result (Rules#8: check all allocation results) */
    if (p == NULL) {
        /* Allocation failed - return NULL to caller (Rules#11: generic error) */
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    /* Set status to 0 explicitly (already zeroed by calloc, but explicit for clarity) */
    p->status = 0;
    
    /* All other fields are already zero-initialized by calloc:
     * - name array is zeroed (null terminated)
     * - age is 0
     */
    
    return p;
}

/* Secure deallocation function
 * Security measures:
 * - Clear sensitive data before free (Rules#9)
 * - Prevent double-free by setting pointer to NULL (Rules#8)
 */
void free_person(person **p) {
    if (p == NULL || *p == NULL) {
        return;
    }
    
    /* Clear sensitive data before freeing (Rules#9)
     * Use memset to zero the structure before deallocation
     * Note: In production, use memset_s if available for guaranteed zeroization
     */
    memset(*p, 0, sizeof(person));
    
    /* Free the memory */
    free(*p);
    
    /* Set pointer to NULL to prevent double-free (Rules#8) */
    *p = NULL;
}

/* Test cases demonstrating secure usage */
int main(void) {
    printf("=== Test Case 1: Basic allocation and verification ===\\n");
    person *p1 = allocate_person();
    if (p1 != NULL) {
        printf("Person allocated successfully\\n");
        printf("Status: %d (expected: 0)\\n", p1->status);
        printf("Age: %d (expected: 0)\\n", p1->age);
        printf("Name[0]: %d (expected: 0)\\n", p1->name[0]);
        free_person(&p1);
        printf("Person freed successfully\\n\\n");
    } else {
        printf("Allocation failed\\n\\n");
    }
    
    printf("=== Test Case 2: Multiple allocations ===\\n");
    person *p2 = allocate_person();
    person *p3 = allocate_person();
    if (p2 != NULL && p3 != NULL) {
        printf("Multiple persons allocated successfully\\n");
        printf("p2 status: %d, p3 status: %d\\n", p2->status, p3->status);
        free_person(&p2);
        free_person(&p3);
        printf("All persons freed successfully\\n\\n");
    } else {
        if (p2 != NULL) free_person(&p2);
        if (p3 != NULL) free_person(&p3);
        printf("One or more allocations failed\\n\\n");
    }
    
    printf("=== Test Case 3: Modify and verify status ===\\n");
    person *p4 = allocate_person();
    if (p4 != NULL) {
        printf("Initial status: %d\\n", p4->status);
        p4->status = 1;
        printf("Modified status: %d\\n", p4->status);
        free_person(&p4);
        printf("Person freed successfully\\n\\n");
    }
    
    printf("=== Test Case 4: Double-free protection ===\\n");
    person *p5 = allocate_person();
    if (p5 != NULL) {
        free_person(&p5);
        printf("First free successful, p5 is now NULL\\n");
        free_person(&p5);  /* Safe - function checks for NULL */
        printf("Second free call handled safely\\n\\n");
    }
    
    printf("=== Test Case 5: NULL pointer handling ===\\n");
    person *p6 = NULL;
    free_person(&p6);  /* Should handle gracefully */
    printf("NULL pointer handled safely\\n");
    
    return 0;
}
