
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char name[50];
    int age;
} Person;

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        person->status = 0;
        memset(person->name, 0, sizeof(person->name));
        person->age = 0;
    }
    return person;
}

int main() {
    /* Test case 1: Basic allocation */
    Person* person1 = allocatePerson();
    printf("Test 1 - Status: %d\\n", person1->status);
    
    /* Test case 2: Multiple allocations */
    Person* person2 = allocatePerson();
    printf("Test 2 - Status: %d\\n", person2->status);
    
    /* Test case 3: Verify independence */
    person1->status = 1;
    Person* person3 = allocatePerson();
    printf("Test 3 - Status: %d\\n", person3->status);
    
    /* Test case 4: Check not null */
    Person* person4 = allocatePerson();
    printf("Test 4 - Not null: %d\\n", (person4 != NULL));
    
    /* Test case 5: Verify initial values */
    Person* person5 = allocatePerson();
    printf("Test 5 - Status: %d, Age: %d\\n", person5->status, person5->age);
    
    /* Free allocated memory */
    free(person1);
    free(person2);
    free(person3);
    free(person4);
    free(person5);
    
    return 0;
}
