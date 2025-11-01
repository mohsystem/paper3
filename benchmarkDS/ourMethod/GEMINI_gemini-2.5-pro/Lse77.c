#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A struct to represent a Person
typedef struct {
    int status;
    char *name;
} Person;

/**
 * @brief Allocates a block of memory for a Person and initializes it.
 *
 * This function performs two memory allocations: one for the struct itself
 * and another for the name string. It includes checks to ensure both
 * allocations succeed.
 *
 * @return A pointer to the newly created Person, or NULL on failure.
 */
Person* createPerson() {
    // 1. Allocate memory for the Person struct
    Person *p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        perror("Failed to allocate memory for Person struct");
        return NULL;
    }

    // Initialize fields to a known state
    p->status = 0;
    p->name = NULL;

    const char* default_name = "John";
    // +1 for the null-terminator
    size_t name_len = strlen(default_name) + 1;

    // 2. Allocate memory for the name string
    p->name = (char*)malloc(name_len);
    if (p->name == NULL) {
        perror("Failed to allocate memory for name");
        free(p); // IMPORTANT: Clean up partially allocated struct to avoid memory leak
        return NULL;
    }

    // 3. Copy the name into the allocated buffer.
    // strcpy is safe here because we calculated and allocated the exact required size.
    strcpy(p->name, default_name);
    
    return p;
}

/**
 * @brief Frees the memory allocated for a Person.
 *
 * It is crucial to free the inner 'name' pointer before freeing the
 * struct pointer itself to avoid orphaning memory.
 *
 * @param p A pointer to the Person to be deallocated.
 */
void destroyPerson(Person *p) {
    if (p != NULL) {
        // Free the allocated string first
        free(p->name);
        p->name = NULL;
        // Then free the struct itself
        free(p);
    }
}

int main() {
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        Person *person = createPerson();

        if (person != NULL) {
            printf("  Person created successfully.\n");
            printf("  Name: %s, Status: %d\n", person->name, person->status);
            destroyPerson(person);
            printf("  Person destroyed successfully.\n");
        } else {
            printf("  Failed to create person.\n");
        }
        printf("\n");
    }
    return 0;
}