
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
    // Test case 1: Allocate a person and check status
    Person* p1 = allocatePerson();
    printf("Test 1 - Status: %d\\n", p1->status);
    
    // Test case 2: Allocate another person and verify independence
    Person* p2 = allocatePerson();
    p2->status = 1;
    printf("Test 2 - P1 Status: %d, P2 Status: %d\\n", p1->status, p2->status);
    
    // Test case 3: Allocate third person
    Person* p3 = allocatePerson();
    printf("Test 3 - Status: %d\\n", p3->status);
    
    // Test case 4: Allocate fourth person
    Person* p4 = allocatePerson();
    printf("Test 4 - Status: %d\\n", p4->status);
    
    // Test case 5: Allocate fifth person
    Person* p5 = allocatePerson();
    printf("Test 5 - Status: %d\\n", p5->status);
    
    // Free allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
