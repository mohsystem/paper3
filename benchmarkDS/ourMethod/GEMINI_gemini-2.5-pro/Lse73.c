#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX_LEN 31 // Max length for name, allowing for a null terminator.

typedef struct {
    char name[NAME_MAX_LEN + 1];
    int age;
} Person;

/**
 * @brief Securely allocates and initializes a block of memory for a Person struct.
 *
 * Allocates a block of memory of size sizeof(Person) and returns a pointer to it.
 * Uses calloc to ensure the memory is zero-initialized, which is a safer default.
 * Checks for allocation failure.
 * Safely copies the name using strncpy to prevent buffer overflows.
 * 
 * @param name The name of the person. Must not be NULL.
 * @param age The age of the person.
 * @return A pointer to the newly allocated Person struct, or NULL on failure.
 *         The caller is responsible for freeing this memory using free().
 */
Person* allocatePerson(const char* name, int age) {
    if (name == NULL) {
        fprintf(stderr, "Error: Input name cannot be NULL.\n");
        return NULL;
    }
    if (age < 0) {
        fprintf(stderr, "Error: Age cannot be negative.\n");
        return NULL;
    }

    // Use calloc for allocation. It allocates memory and initializes it to zero.
    // This is generally safer than malloc as it prevents use of uninitialized memory.
    Person* p = (Person*)calloc(1, sizeof(Person));

    // CRITICAL: Always check the return value of allocation functions.
    if (p == NULL) {
        perror("FATAL: Failed to allocate memory for Person");
        return NULL;
    }

    // Initialize the struct members.
    p->age = age;

    // Use strncpy for bounded string copy to prevent buffer overflows.
    strncpy(p->name, name, NAME_MAX_LEN);
    
    // Ensure null-termination, as strncpy might not null-terminate
    // if the source string is as long as or longer than the destination buffer.
    p->name[NAME_MAX_LEN] = '\0';

    return p;
}

/**
 * @brief Frees the memory allocated for a Person object.
 *
 * @param p A pointer to the Person object to be freed. Can be NULL.
 */
void freePerson(Person* p) {
    // free(NULL) is a no-op, so this check is not strictly necessary,
    // but can be good practice for clarity and preventing double-free logic bugs.
    if (p != NULL) {
        free(p);
    }
}

int main(void) {
    printf("--- Running C Test Cases ---\n");
    Person* people[5] = {NULL}; // Initialize all pointers to NULL.
    int allocated_count = 0;

    // Test Case 1
    people[0] = allocatePerson("Alice", 30);
    if (people[0] != NULL) {
        printf("Test Case 1: Allocated Person{name='%s', age=%d}\n", people[0]->name, people[0]->age);
        allocated_count++;
    }

    // Test Case 2
    people[1] = allocatePerson("Bob", 25);
    if (people[1] != NULL) {
        printf("Test Case 2: Allocated Person{name='%s', age=%d}\n", people[1]->name, people[1]->age);
        allocated_count++;
    }

    // Test Case 3
    people[2] = allocatePerson("Charlie", 35);
    if (people[2] != NULL) {
        printf("Test Case 3: Allocated Person{name='%s', age=%d}\n", people[2]->name, people[2]->age);
        allocated_count++;
    }

    // Test Case 4
    people[3] = allocatePerson("Diana", 28);
    if (people[3] != NULL) {
        printf("Test Case 4: Allocated Person{name='%s', age=%d}\n", people[3]->name, people[3]->age);
        allocated_count++;
    }
    
    // Test Case 5
    people[4] = allocatePerson("Eve", 40);
    if (people[4] != NULL) {
        printf("Test Case 5: Allocated Person{name='%s', age=%d}\n", people[4]->name, people[4]->age);
        allocated_count++;
    }

    printf("Total people allocated: %d\n", allocated_count);
    
    // IMPORTANT: Free all allocated memory to prevent memory leaks.
    printf("Freeing allocated memory...\n");
    for (int i = 0; i < 5; ++i) {
        freePerson(people[i]);
        people[i] = NULL; // Good practice to nullify pointer after freeing.
    }
    
    printf("--- C Test Cases Complete ---\n");
    return 0;
}