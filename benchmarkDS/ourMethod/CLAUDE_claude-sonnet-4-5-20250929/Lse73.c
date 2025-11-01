
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Person structure with validated fields */
typedef struct {
    char* name;
    int age;
    char* address;
} Person;

/* Secure memory zeroing function that compiler cannot optimize away */
static void secure_zero(void* ptr, size_t len) {
    if (!ptr || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Memory allocation function that allocates a Person structure */
/* Returns a pointer to allocated memory or NULL on failure */
Person* allocatePerson(void) {
    /* Allocate memory using calloc for automatic zero initialization */
    Person* person = (Person*)calloc(1, sizeof(Person));
    
    /* Check allocation result - must validate before use */
    if (person == NULL) {
        fprintf(stderr, "Memory allocation failed for Person structure\\n");
        return NULL;
    }
    
    /* Allocate memory for name field with size validation */
    /* 100 chars + 1 for null terminator */
    person->name = (char*)calloc(101, sizeof(char));
    if (person->name == NULL) {
        fprintf(stderr, "Memory allocation failed for name field\\n");
        free(person);
        return NULL;
    }
    
    /* Allocate memory for address field with size validation */
    /* 200 chars + 1 for null terminator */
    person->address = (char*)calloc(201, sizeof(char));
    if (person->address == NULL) {
        fprintf(stderr, "Memory allocation failed for address field\\n");
        secure_zero(person->name, 101);
        free(person->name);
        free(person);
        return NULL;
    }
    
    /* Initialize age to 0 (already done by calloc but explicit for clarity) */
    person->age = 0;
    
    return person;
}

/* Overloaded-style function to allocate and initialize Person */
Person* allocatePersonWithData(const char* name, int age, const char* address) {
    /* Input validation - check for NULL pointers */
    if (name == NULL || address == NULL) {
        fprintf(stderr, "Invalid input: null pointer provided\\n");
        return NULL;
    }
    
    /* Validate string lengths to prevent buffer overflow */
    size_t nameLen = strlen(name);
    size_t addrLen = strlen(address);
    
    if (nameLen > 100) {
        fprintf(stderr, "Invalid input: name exceeds maximum length\\n");
        return NULL;
    }
    
    if (addrLen > 200) {
        fprintf(stderr, "Invalid input: address exceeds maximum length\\n");
        return NULL;
    }
    
    /* Validate age range */
    if (age < 0 || age > 150) {
        fprintf(stderr, "Invalid input: age out of valid range\\n");
        return NULL;
    }
    
    /* Allocate Person structure */
    Person* person = allocatePerson();
    if (person == NULL) {
        return NULL;
    }
    
    /* Safe string copy with explicit null termination */
    strncpy(person->name, name, 100);
    person->name[100] = '\\0'; /* Ensure null termination */
    
    person->age = age;
    
    strncpy(person->address, address, 200);
    person->address[200] = '\\0'; /* Ensure null termination */
    
    return person;
}

/* Function to safely deallocate Person - frees memory exactly once */
void deallocatePerson(Person* person) {
    if (person == NULL) {
        return;
    }
    
    /* Clear sensitive data before freeing */
    if (person->name != NULL) {
        secure_zero(person->name, 101);
        free(person->name);
        person->name = NULL;
    }
    
    if (person->address != NULL) {
        secure_zero(person->address, 201);
        free(person->address);
        person->address = NULL;
    }
    
    /* Clear the structure itself */
    secure_zero(person, sizeof(Person));
    free(person);
}

int main(void) {
    /* Test case 1: Basic allocation */
    printf("Test 1: Basic allocation\\n");
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Person allocated: Success\\n");
        deallocatePerson(p1);
        p1 = NULL;
    } else {
        printf("Person allocation: Failed\\n");
    }
    
    /* Test case 2: Allocation with initialization */
    printf("\\nTest 2: Allocation with initialization\\n");
    Person* p2 = allocatePersonWithData("John Doe", 30, "123 Main St");
    if (p2 != NULL) {
        printf("Name: %s, Age: %d, Address: %s\\n", 
               p2->name, p2->age, p2->address);
        deallocatePerson(p2);
        p2 = NULL;
    }
    
    /* Test case 3: Multiple allocations */
    printf("\\nTest 3: Multiple allocations\\n");
    Person* p3 = allocatePersonWithData("Alice Smith", 25, "456 Oak Ave");
    Person* p4 = allocatePersonWithData("Bob Johnson", 45, "789 Pine Rd");
    if (p3 != NULL && p4 != NULL) {
        printf("p3 - Name: %s, Age: %d\\n", p3->name, p3->age);
        printf("p4 - Name: %s, Age: %d\\n", p4->name, p4->age);
        deallocatePerson(p3);
        deallocatePerson(p4);
        p3 = NULL;
        p4 = NULL;
    }
    
    /* Test case 4: Invalid input handling - NULL pointer */
    printf("\\nTest 4: Invalid input handling\\n");
    Person* p5 = allocatePersonWithData(NULL, 30, "Address");
    if (p5 == NULL) {
        printf("Correctly rejected null input\\n");
    }
    
    /* Test case 5: Boundary value testing - invalid age */
    printf("\\nTest 5: Boundary value testing\\n");
    Person* p6 = allocatePersonWithData("Valid Name", -5, "Address");
    if (p6 == NULL) {
        printf("Correctly rejected invalid age\\n");
    }
    
    Person* p7 = allocatePersonWithData("Edge Case", 0, "Valid Address");
    if (p7 != NULL) {
        printf("Allocated person with age 0: Success\\n");
        deallocatePerson(p7);
        p7 = NULL;
    }
    
    return 0;
}
