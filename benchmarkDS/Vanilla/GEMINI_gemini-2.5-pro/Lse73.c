#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the person structure
typedef struct {
    char name[50];
    int age;
} Person;

/**
 * Allocates a block of memory of size sizeof(Person) using malloc.
 *
 * @return A pointer to the beginning of the allocated block, or NULL on failure.
 */
Person* allocatePerson() {
    Person* p = (Person*)malloc(sizeof(Person));
    return p;
}

int main() {
    printf("C Test Cases:\n");

    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        Person* p = allocatePerson();
        if (p != NULL) {
            // Initialize the allocated object
            sprintf(p->name, "Person %d", i);
            p->age = 20 + i;
            printf("Successfully allocated and initialized: Person{name='%s', age=%d}\n", p->name, p->age);
            
            // Deallocate the memory to prevent memory leaks
            free(p);
            printf("Memory deallocated.\n");
        } else {
            printf("Memory allocation failed.\n");
        }
    }
    return 0;
}