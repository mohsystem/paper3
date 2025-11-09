
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define the person structure */
typedef struct {
    int status;
    char name[64];
    int age;
} person;

/* Function to allocate and initialize a person structure
 * Security considerations:
 * - Check malloc result for NULL (memory exhaustion)
 * - Initialize all fields to prevent information leakage
 * - Use calloc for automatic zero-initialization
 */
person* allocate_person(void) {
    /* Use calloc instead of malloc to ensure zero-initialization
     * This prevents uninitialized memory from leaking sensitive data
     * Rules#8: Initialize pointers and check all allocation results
     */
    person* p = (person*)calloc(1, sizeof(person));
    
    /* Check if allocation succeeded - Rules#8 */
    if (p == NULL) {
        /* Rules#11: Return NULL on failure without leaking internal details */
        return NULL;
    }
    
    /* Explicitly set status to 0 (redundant with calloc but explicit per requirement)
     * Rules#8: Ensure all fields are properly initialized
     */
    p->status = 0;
    
    /* calloc already zeroed the memory, but being explicit about initialization */
    p->age = 0;
    /* name array is already zeroed by calloc, ensuring null termination */
    
    return p;
}

/* Function to safely free a person structure
 * Rules#8: Free memory exactly once
 * Rules#9: Clear sensitive data before free if needed
 */
void free_person(person** p) {
    if (p == NULL || *p == NULL) {
        return;
    }
    
    /* If person contained sensitive data, we would clear it here
     * For this example, clearing the structure before free
     * Rules#9: Clear sensitive data before free
     */
    memset(*p, 0, sizeof(person));
    
    free(*p);
    *p = NULL; /* Prevent double-free by nulling the pointer */
}

/* Main function with test cases */
int main(void) {
    person* test_person = NULL;
    
    printf("Test Case 1: Basic allocation\\n");
    test_person = allocate_person();
    if (test_person != NULL) {
        printf("  Success: Person allocated, status = %d\\n", test_person->status);
        free_person(&test_person);
    } else {
        printf("  Failed: Allocation returned NULL\\n");
    }
    
    printf("\\nTest Case 2: Verify status is 0\\n");
    test_person = allocate_person();
    if (test_person != NULL) {
        if (test_person->status == 0) {
            printf("  Success: Status correctly initialized to 0\\n");
        } else {
            printf("  Failed: Status is %d, expected 0\\n", test_person->status);
        }
        free_person(&test_person);
    }
    
    printf("\\nTest Case 3: Verify all fields are zero-initialized\\n");
    test_person = allocate_person();
    if (test_person != NULL) {
        int all_zero = 1;
        for (size_t i = 0; i < sizeof(test_person->name); i++) {
            if (test_person->name[i] != 0) {
                all_zero = 0;
                break;
            }
        }
        if (all_zero && test_person->age == 0 && test_person->status == 0) {
            printf("  Success: All fields properly zero-initialized\\n");
        } else {
            printf("  Failed: Some fields not zero-initialized\\n");
        }
        free_person(&test_person);
    }
    
    printf("\\nTest Case 4: Multiple allocations\\n");
    person* person1 = allocate_person();
    person* person2 = allocate_person();
    person* person3 = allocate_person();
    if (person1 != NULL && person2 != NULL && person3 != NULL) {
        printf("  Success: Multiple persons allocated successfully\\n");
        printf("  Person1 status: %d\\n", person1->status);
        printf("  Person2 status: %d\\n", person2->status);
        printf("  Person3 status: %d\\n", person3->status);
    } else {
        printf("  Failed: One or more allocations failed\\n");
    }
    free_person(&person1);
    free_person(&person2);
    free_person(&person3);
    
    printf("\\nTest Case 5: Safe double-free prevention\\n");
    test_person = allocate_person();
    if (test_person != NULL) {
        free_person(&test_person);
        /* This should be safe due to NULL pointer check */
        free_person(&test_person);
        printf("  Success: Double-free prevented, no crash\\n");
    }
    
    return 0;
}
