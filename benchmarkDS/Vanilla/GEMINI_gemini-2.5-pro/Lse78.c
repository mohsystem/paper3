#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
    // You can add other person attributes here
    // char* name;
    // int age;
} Person;

/**
 * Allocates a new Person object on the heap and sets its status to 0.
 * @return A pointer to the newly allocated Person object, or NULL on failure.
 */
Person* allocatePerson() {
    // Allocate a block of memory of the size of a Person struct
    Person* newPerson = (Person*)malloc(sizeof(Person));

    // Check if malloc was successful
    if (newPerson != NULL) {
        // Set the status to 0
        newPerson->status = 0;
    }
    
    // Return the pointer to the beginning of the block
    return newPerson;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Test Case 1: Person allocated with status: %d\n", p1->status);
        free(p1); // Free the allocated memory
    } else {
        fprintf(stderr, "Test Case 1: Memory allocation failed!\n");
    }

    // Test Case 2
    Person* p2 = allocatePerson();
    if (p2 != NULL) {
        printf("Test Case 2: Person allocated with status: %d\n", p2->status);
        free(p2);
    } else {
        fprintf(stderr, "Test Case 2: Memory allocation failed!\n");
    }

    // Test Case 3
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        printf("Test Case 3: Person allocated with status: %d\n", p3->status);
        free(p3);
    } else {
        fprintf(stderr, "Test Case 3: Memory allocation failed!\n");
    }

    // Test Case 4
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        printf("Test Case 4: Person allocated with status: %d\n", p4->status);
        free(p4);
    } else {
        fprintf(stderr, "Test Case 4: Memory allocation failed!\n");
    }

    // Test Case 5
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        printf("Test Case 5: Person allocated with status: %d\n", p5->status);
        free(p5);
    } else {
        fprintf(stderr, "Test Case 5: Memory allocation failed!\n");
    }

    return 0;
}