
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char name[50];
    int age;
} Person;

Person* allocatePerson() {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p != NULL) {
        p->status = 0;
        memset(p->name, 0, sizeof(p->name));
        p->age = 0;
    }
    return p;
}

int main() {
    // Test case 1: Allocate a person and verify status is 0
    Person* person1 = allocatePerson();
    printf("Test 1 - Status: %d\\n", person1->status);
    
    // Test case 2: Allocate another person and verify status is 0
    Person* person2 = allocatePerson();
    printf("Test 2 - Status: %d\\n", person2->status);
    
    // Test case 3: Modify status and allocate new person
    person1->status = 5;
    Person* person3 = allocatePerson();
    printf("Test 3 - Person1 Status: %d, Person3 Status: %d\\n", 
           person1->status, person3->status);
    
    // Test case 4: Verify multiple allocations
    Person* person4 = allocatePerson();
    Person* person5 = allocatePerson();
    printf("Test 4 - Person4 Status: %d, Person5 Status: %d\\n", 
           person4->status, person5->status);
    
    // Test case 5: Verify pointers are different
    printf("Test 5 - Are person1 and person2 different objects? %s\\n", 
           (person1 != person2) ? "true" : "false");
    
    // Free allocated memory
    free(person1);
    free(person2);
    free(person3);
    free(person4);
    free(person5);
    
    return 0;
}
