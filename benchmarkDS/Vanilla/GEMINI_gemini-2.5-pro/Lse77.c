#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char* name;
} Person;

/**
 * Allocates a block of memory for a Person struct using malloc
 * and returns a pointer to the beginning of the block.
 * @return A pointer to a new Person struct with name "John" and status 0.
 *         Returns NULL if memory allocation fails.
 */
Person* createPerson() {
    // Allocate memory for the Person struct itself.
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        return NULL; // Allocation failed
    }

    // Allocate memory for the name string.
    // "John" is 4 chars + 1 for the null terminator.
    p->name = (char*)malloc(5 * sizeof(char));
    if (p->name == NULL) {
        free(p); // Free the previously allocated struct memory
        return NULL; // Allocation failed
    }
    
    // Sets the status to 0 and the name to "John".
    p->status = 0;
    strcpy(p->name, "John");

    return p;
}

/**
 * Frees the memory allocated for a Person struct, including its name.
 * @param p A pointer to the Person struct to be deallocated.
 */
void destroyPerson(Person* p) {
    if (p != NULL) {
        free(p->name); // Free the name string first
        free(p);       // Then free the struct itself
    }
}

int main() {
    // In C, a main class does not exist. We use the main function directly.
    printf("Running C Test Cases:\n");

    // Test Case 1
    Person* p1 = createPerson();
    if (p1 != NULL) {
        printf("Test Case 1: Person{name='%s', status=%d}\n", p1->name, p1->status);
        destroyPerson(p1);
    }

    // Test Case 2
    Person* p2 = createPerson();
    if (p2 != NULL) {
        printf("Test Case 2: Person{name='%s', status=%d}\n", p2->name, p2->status);
        destroyPerson(p2);
    }
    
    // Test Case 3
    Person* p3 = createPerson();
    if (p3 != NULL) {
        printf("Test Case 3: Person{name='%s', status=%d}\n", p3->name, p3->status);
        destroyPerson(p3);
    }

    // Test Case 4
    Person* p4 = createPerson();
    if (p4 != NULL) {
        printf("Test Case 4: Person{name='%s', status=%d}\n", p4->name, p4->status);
        destroyPerson(p4);
    }

    // Test Case 5
    Person* p5 = createPerson();
    if (p5 != NULL) {
        printf("Test Case 5: Person{name='%s', status=%d}\n", p5->name, p5->status);
        destroyPerson(p5);
    }

    return 0;
}