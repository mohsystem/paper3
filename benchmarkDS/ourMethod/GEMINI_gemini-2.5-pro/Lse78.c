#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define a maximum name length to prevent buffer overflows
#define MAX_NAME_LEN 63

typedef struct {
    int status;
    char name[MAX_NAME_LEN + 1]; // +1 for null terminator
} Person;

/**
 * @brief Securely deallocates a Person object.
 *
 * Sets the pointer to NULL after freeing to prevent use-after-free vulnerabilities.
 *
 * @param person_ptr A pointer to the Person pointer to be freed.
 */
void destroyPerson(Person** person_ptr) {
    if (person_ptr != NULL && *person_ptr != NULL) {
        // For sensitive data, the memory should be securely zeroed before freeing.
        // e.g., using memset_s if available, or a custom volatile loop.
        free(*person_ptr);
        *person_ptr = NULL;
    }
}

/**
 * @brief Allocates and initializes a new Person object.
 *
 * This function allocates a block of memory for a Person, sets the status to 0,
 * and safely copies the provided name, preventing buffer overflows.
 *
 * @param name The name to assign to the new person. Cannot be NULL.
 * @return A pointer to the newly allocated Person, or NULL on failure.
 */
Person* createPerson(const char* name) {
    if (name == NULL) {
        fprintf(stderr, "Error: Name cannot be NULL.\n");
        return NULL;
    }

    // Allocate memory for the Person struct
    Person* new_person = (Person*)malloc(sizeof(Person));
    if (new_person == NULL) {
        // Check for malloc failure
        perror("Error: Memory allocation failed");
        return NULL;
    }

    // Initialize the person's status to 0
    new_person->status = 0;

    // Safely copy the name using strncpy to prevent buffer overflows
    strncpy(new_person->name, name, MAX_NAME_LEN);
    // Ensure the string is null-terminated, as strncpy might not if source is too long
    new_person->name[MAX_NAME_LEN] = '\0';

    return new_person;
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    
    Person* people[5] = {NULL}; // Array of pointers, initialized to NULL

    // Test Case 1
    people[0] = createPerson("Alice");
    if (people[0] != NULL) {
        printf("Test Case 1: Created Person{name='%s', status=%d}\n", people[0]->name, people[0]->status);
    }

    // Test Case 2
    people[1] = createPerson("Bob");
    if (people[1] != NULL) {
        printf("Test Case 2: Created Person{name='%s', status=%d}\n", people[1]->name, people[1]->status);
    }

    // Test Case 3: A name longer than the buffer to test truncation
    const char* long_name = "Charlie-has-a-very-long-name-that-will-be-truncated-by-the-function";
    people[2] = createPerson(long_name);
    if (people[2] != NULL) {
        printf("Test Case 3: Created Person{name='%s', status=%d} (Name was truncated)\n", people[2]->name, people[2]->status);
    }

    // Test Case 4
    people[3] = createPerson("Diana");
    if (people[3] != NULL) {
        printf("Test Case 4: Created Person{name='%s', status=%d}\n", people[3]->name, people[3]->status);
    }

    // Test Case 5: Invalid input
    people[4] = createPerson(NULL);
    if (people[4] == NULL) {
        printf("Test Case 5: Failed to create person as expected for NULL input.\n");
    }

    // Cleanup: deallocate all created persons
    for (int i = 0; i < 5; ++i) {
        destroyPerson(&people[i]);
    }

    printf("\n--- Test Cases Finished ---\n");

    return 0;
}