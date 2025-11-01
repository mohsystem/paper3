#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a structure for a person.
// Using a fixed-size array for the name is common in C.
#define NAME_SIZE 50
typedef struct {
    int status;
    char name[NAME_SIZE];
} Person;

/**
 * Allocates a block of memory for a new Person using malloc.
 * Initializes the Person's status to 0 and name to "John".
 *
 * @return A pointer to the newly allocated Person struct, or NULL on failure.
 */
Person* createPerson() {
    // Allocate a block of memory of the size of one Person struct.
    Person* p = (Person*)malloc(sizeof(Person));

    // Security: Always check if malloc returned NULL.
    // This happens if the system is out of memory.
    if (p == NULL) {
        perror("Unable to allocate memory for Person");
        return NULL;
    }

    // Initialize the allocated memory.
    p->status = 0;
    
    // Security: Use strncpy to prevent buffer overflows.
    // It copies at most NAME_SIZE - 1 characters.
    strncpy(p->name, "John", NAME_SIZE - 1);
    
    // Security: Ensure the string is null-terminated, as strncpy might not
    // add a null terminator if the source string is as long as the destination buffer.
    p->name[NAME_SIZE - 1] = '\0';
    
    return p;
}

int main() {
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        Person* p = createPerson();

        // Security: Always check for a NULL pointer before dereferencing it.
        if (p != NULL) {
            printf("  Successfully created person at address %p: Status=%d, Name='%s'\n", (void*)p, p->status, p->name);
            
            // Security: Free the allocated memory to prevent memory leaks.
            free(p);
            p = NULL; // Best practice: avoid use-after-free by nulling the pointer.
            printf("  Memory freed.\n");
        } else {
            printf("  Failed to create person.\n");
        }
    }
    return 0;
}