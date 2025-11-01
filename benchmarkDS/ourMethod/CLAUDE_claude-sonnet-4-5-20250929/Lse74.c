
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Person structure with status field */
typedef struct {
    int status;
} Person;

/**
 * Allocates memory for a Person structure and initializes status to 0.
 * 
 * Returns a pointer to the allocated Person, or NULL on failure.
 * Caller is responsible for freeing the returned pointer.
 */
Person* allocatePerson(void) {
    /* Allocate memory for Person structure using malloc */
    Person* person = (Person*)malloc(sizeof(Person));
    
    /* Check if allocation succeeded (critical security check) */
    if (person == NULL) {
        /* Allocation failed, return NULL (fail closed) */
        return NULL;
    }
    
    /* Initialize the entire structure to zero for safety */
    /* This prevents use of uninitialized memory */
    memset(person, 0, sizeof(Person));
    
    /* Explicitly set status field to 0 (redundant but matches specification) */
    person->status = 0;
    
    return person;
}

/**
 * Safely deallocates a Person structure.
 * Sets the pointer to NULL after freeing to prevent use-after-free.
 */
void deallocatePerson(Person** person) {
    if (person != NULL && *person != NULL) {
        /* No sensitive data to clear in this simple structure */
        /* If the structure contained secrets, we would use memset_s or explicit_bzero here */
        free(*person);
        *person = NULL;  /* Prevent double-free and use-after-free */
    }
}

int main(void) {
    printf("Testing Person allocation:\\n");
    
    /* Test case 1: Allocate a single person */
    Person* person1 = allocatePerson();
    if (person1 != NULL) {
        printf("Test 1 - Person allocated with status: %d\\n", person1->status);
        deallocatePerson(&person1);
    } else {
        printf("Test 1 - Allocation failed\\n");
        return 1;
    }
    
    /* Test case 2: Allocate another person */
    Person* person2 = allocatePerson();
    if (person2 != NULL) {
        printf("Test 2 - Person allocated with status: %d\\n", person2->status);
        deallocatePerson(&person2);
    } else {
        printf("Test 2 - Allocation failed\\n");
        return 1;
    }
    
    /* Test case 3: Verify status is 0 */
    Person* person3 = allocatePerson();
    if (person3 != NULL) {
        printf("Test 3 - Status is 0: %s\\n", (person3->status == 0) ? "true" : "false");
        deallocatePerson(&person3);
    } else {
        printf("Test 3 - Allocation failed\\n");
        return 1;
    }
    
    /* Test case 4: Allocate multiple persons */
    const int count = 5;
    Person* people[5] = {NULL};  /* Initialize all pointers to NULL */
    int allocated = 0;
    for (int i = 0; i < count; i++) {
        people[i] = allocatePerson();
        if (people[i] != NULL) {
            allocated++;
        }
    }
    printf("Test 4 - Allocated %d persons\\n", allocated);
    /* Free all allocated persons */
    for (int i = 0; i < count; i++) {
        deallocatePerson(&people[i]);
    }
    
    /* Test case 5: Verify all have status 0 */
    Person* person4 = allocatePerson();
    Person* person5 = allocatePerson();
    if (person4 != NULL && person5 != NULL) {
        int allZero = (person4->status == 0) && (person5->status == 0);
        printf("Test 5 - All status fields are 0: %s\\n", allZero ? "true" : "false");
    } else {
        printf("Test 5 - Allocation failed\\n");
    }
    deallocatePerson(&person4);
    deallocatePerson(&person5);
    
    return 0;
}
