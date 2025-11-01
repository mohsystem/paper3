
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
    if (person == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    person->status = 0;
    memset(person->name, 0, sizeof(person->name));
    person->age = 0;
    
    return person;
}

void freePerson(Person* person) {
    if (person != NULL) {
        free(person);
    }
}

int main() {
    // Test case 1: Normal allocation
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Test 1 - Person allocated, status: %d\\n", p1->status);
    }
    
    // Test case 2: Verify status is 0
    Person* p2 = allocatePerson();
    if (p2 != NULL) {
        printf("Test 2 - Status is zero: %d\\n", p2->status == 0);
    }
    
    // Test case 3: Multiple allocations
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        printf("Test 3 - Multiple allocation successful, status: %d\\n", p3->status);
    }
    
    // Test case 4: Modify and verify independence
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        p4->status = 5;
        printf("Test 4 - Modified status: %d\\n", p4->status);
    }
    
    // Test case 5: Verify new allocation is still 0
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        printf("Test 5 - New allocation status still 0: %d\\n", p5->status == 0);
    }
    
    // Clean up
    freePerson(p1);
    freePerson(p2);
    freePerson(p3);
    freePerson(p4);
    freePerson(p5);
    
    return 0;
}
