
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

// Maximum name length to prevent excessive memory allocation
#define MAX_NAME_LENGTH 255
#define MAX_STATUS 1000

// Person structure
typedef struct {
    int status;
    char* name;
    size_t name_capacity;
} Person;

// Function to securely zero memory
static void secure_zero(void* ptr, size_t size) {
    if (ptr == NULL || size == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

// Function to create and initialize a Person
Person* createPerson(int status, const char* name) {
    // Input validation: check name is not null
    if (name == NULL) {
        fprintf(stderr, "Error: Name cannot be null\\n");
        return NULL;
    }
    
    // Input validation: check status range
    if (status < 0 || status > MAX_STATUS) {
        fprintf(stderr, "Error: Status must be between 0 and %d\\n", MAX_STATUS);
        return NULL;
    }
    
    // Check name length to prevent excessive allocation
    size_t name_len = strnlen(name, MAX_NAME_LENGTH + 1);
    if (name_len > MAX_NAME_LENGTH) {
        fprintf(stderr, "Error: Name exceeds maximum length of %d\\n", MAX_NAME_LENGTH);
        return NULL;
    }
    
    // Allocate Person structure
    Person* person = (Person*)calloc(1, sizeof(Person));
    if (person == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for Person\\n");
        return NULL;
    }
    
    // Calculate name buffer size with overflow check
    size_t name_capacity = name_len + 1;
    if (name_capacity < name_len) {
        fprintf(stderr, "Error: Integer overflow in size calculation\\n");
        free(person);
        return NULL;
    }
    
    // Allocate memory for name with calloc for zero initialization
    person->name = (char*)calloc(name_capacity, sizeof(char));
    if (person->name == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for name\\n");
        free(person);
        return NULL;
    }
    
    person->name_capacity = name_capacity;
    
    // Safe string copy with bounds checking
    strncpy(person->name, name, name_capacity - 1);
    person->name[name_capacity - 1] = '\\0';  // Ensure null termination
    
    person->status = status;
    
    return person;
}

// Function to free Person and clear sensitive data
void freePerson(Person* person) {
    if (person == NULL) return;
    
    if (person->name != NULL) {
        // Securely zero memory before freeing
        secure_zero(person->name, person->name_capacity);
        free(person->name);
        person->name = NULL;
    }
    
    person->name_capacity = 0;
    person->status = 0;
    
    free(person);
}

// Getters
int getStatus(const Person* person) {
    return (person != NULL) ? person->status : -1;
}

const char* getName(const Person* person) {
    return (person != NULL) ? person->name : NULL;
}

int main(void) {
    Person* person = NULL;
    
    // Test case 1: Create person with status 0 and name "John"
    printf("Test 1:\\n");
    person = createPerson(0, "John");
    if (person != NULL) {
        printf("Status: %d, Name: %s\\n", getStatus(person), getName(person));
        freePerson(person);
    }
    
    // Test case 2: Create person with different status
    printf("\\nTest 2:\\n");
    person = createPerson(1, "Alice");
    if (person != NULL) {
        printf("Status: %d, Name: %s\\n", getStatus(person), getName(person));
        freePerson(person);
    }
    
    // Test case 3: Create person with empty name
    printf("\\nTest 3:\\n");
    person = createPerson(0, "");
    if (person != NULL) {
        printf("Status: %d, Name: '%s'\\n", getStatus(person), getName(person));
        freePerson(person);
    }
    
    // Test case 4: Attempt to create with null name (should fail)
    printf("\\nTest 4:\\n");
    person = createPerson(0, NULL);
    if (person == NULL) {
        printf("Correctly rejected null name\\n");
    }
    
    // Test case 5: Attempt to create with invalid status (should fail)
    printf("\\nTest 5:\\n");
    person = createPerson(-1, "Bob");
    if (person == NULL) {
        printf("Correctly rejected invalid status\\n");
    }
    
    return 0;
}
