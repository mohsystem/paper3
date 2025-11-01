
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
    // Test case 1: Allocate single person
    Person* p1 = allocatePerson();
    if (p1) {
        printf("Test 1 - Person status: %d\\n", p1->status);
    }
    
    // Test case 2: Allocate multiple persons
    Person* p2 = allocatePerson();
    Person* p3 = allocatePerson();
    if (p2 && p3) {
        printf("Test 2 - Person2 status: %d, Person3 status: %d\\n", 
               p2->status, p3->status);
    }
    
    // Test case 3: Verify independence
    if (p1) {
        p1->status = 5;
        Person* p4 = allocatePerson();
        if (p4) {
            printf("Test 3 - Person1 status: %d, Person4 status: %d\\n", 
                   p1->status, p4->status);
            free(p4);
        }
    }
    
    // Test case 4: Allocate and set properties
    Person* p5 = allocatePerson();
    if (p5) {
        strcpy(p5->name, "John");
        p5->age = 30;
        printf("Test 4 - Person5 status: %d, Name: %s\\n", p5->status, p5->name);
        free(p5);
    }
    
    // Test case 5: Multiple allocations in loop
    for (int i = 0; i < 3; i++) {
        Person* p = allocatePerson();
        if (p) {
            printf("Test 5 - Iteration %d status: %d\\n", i, p->status);
            free(p);
        }
    }
    
    // Clean up
    if (p1) free(p1);
    if (p2) free(p2);
    if (p3) free(p3);
    
    return 0;
}
