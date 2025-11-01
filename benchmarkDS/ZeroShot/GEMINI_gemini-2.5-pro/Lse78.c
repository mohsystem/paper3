#include <stdio.h>
#include <stdlib.h>

// A simple structure to represent a Person.
typedef struct {
    int status;
    int id; // Added for testing purposes
} Person;

/**
 * Allocates a block of memory for a new Person.
 *
 * This function uses calloc, which is generally more secure than malloc for this
 * purpose because it initializes the allocated memory to zero. This prevents
 * security vulnerabilities arising from the use of uninitialized memory.
 * Here, it automatically sets 'status' and 'id' to 0.
 *
 * @param id The identifier for the new person.
 * @return A pointer to the newly allocated Person, or NULL if allocation fails.
 */
Person* allocatePerson(int id) {
    // Use calloc to allocate and zero-initialize memory.
    // 1 is the number of elements, sizeof(Person) is the size of each element.
    Person* p = (Person*)calloc(1, sizeof(Person));

    // Secure programming practice: always check if allocation was successful.
    if (p == NULL) {
        // Error handling: allocation failed. Return NULL to the caller.
        return NULL;
    }

    // Although calloc zeroed the memory (status is 0), we set the id.
    p->id = id;
    // Status is already 0 from calloc.
    // p->status = 0; // This line would be redundant but harmless.

    return p;
}

int main() {
    // Use an array of pointers to manage the allocations for cleanup.
    Person* people[5];
    int allocated_count = 0;
    
    printf("Running 5 test cases...\n");

    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        int person_id = i + 1;
        Person* p = allocatePerson(person_id);

        // Securely check the return value of the allocation function.
        if (p != NULL) {
            people[allocated_count++] = p;
            printf("Successfully allocated: Person{id=%d, status=%d}\n", p->id, p->status);
        } else {
            // Use stderr for error messages.
            fprintf(stderr, "Memory allocation failed for person with id %d\n", person_id);
        }
    }

    // Secure programming practice: free all dynamically allocated memory.
    printf("\nCleaning up allocated memory...\n");
    for (int i = 0; i < allocated_count; ++i) {
        free(people[i]);
        people[i] = NULL; // Good practice to nullify pointer after freeing.
    }

    printf("Program finished.\n");
    return 0;
}