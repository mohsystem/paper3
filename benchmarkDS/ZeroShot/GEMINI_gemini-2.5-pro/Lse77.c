#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a buffer size for the name that is reasonably larger than needed.
#define NAME_BUFFER_SIZE 64

typedef struct {
    int status;
    char name[NAME_BUFFER_SIZE];
} Person;

/**
 * @brief Securely allocates a new Person object on the heap.
 * The caller is responsible for deallocating the returned object using 'free()'.
 * @return Person* A pointer to the newly allocated Person, or NULL on failure.
 */
Person* createPerson() {
    // Allocate a block of memory for one Person struct.
    Person* person = (Person*)malloc(sizeof(Person));

    // SECURITY: Always check if malloc returned NULL to handle allocation failures.
    if (person == NULL) {
        perror("Failed to allocate memory for Person");
        return NULL;
    }

    person->status = 0;

    // SECURITY: Use strncpy to prevent buffer overflows. It copies at most
    // (NAME_BUFFER_SIZE - 1) characters, leaving space for the null terminator.
    strncpy(person->name, "John", NAME_BUFFER_SIZE - 1);
    
    // SECURITY: Explicitly null-terminate the string. strncpy does not guarantee
    // null-termination if the source string is as long as the size limit.
    person->name[NAME_BUFFER_SIZE - 1] = '\0';

    return person;
}

int main() {
    printf("--- Running C Test Cases ---\n");
    Person* people[5] = {NULL}; // Array to hold pointers for later cleanup

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        people[i] = createPerson();

        // SECURITY: Always check for a NULL pointer before dereferencing it.
        if (people[i] != NULL) {
            printf("  Created: Person[name=%s, status=%d]\n", people[i]->name, people[i]->status);
            printf("  Memory Address: %p\n", (void*)people[i]);
        } else {
            fprintf(stderr, "  ERROR: Person creation failed in test case %d.\n", i + 1);
        }
    }

    printf("\n--- Cleaning up allocated memory ---\n");
    for (int i = 0; i < 5; ++i) {
        if (people[i] != NULL) {
            // SECURITY: Deallocate the memory to prevent memory leaks.
            free(people[i]);
            // SECURITY: Set the pointer to NULL after freeing to prevent a
            // 'dangling pointer' and potential use-after-free vulnerabilities.
            people[i] = NULL;
        }
    }
    printf("--- C Test Cases Complete ---\n");

    return 0;
}