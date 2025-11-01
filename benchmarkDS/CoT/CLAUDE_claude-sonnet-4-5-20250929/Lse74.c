
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
        person->name[0] = '\\0';
        person->age = 0;
    } else {
        fprintf(stderr, "Memory allocation failed\\n");
    }
    return person;
}

void freePerson(Person* person) {
    if (person != NULL) {
        free(person);
    }
}

int main() {
    printf("Test Case 1: Basic allocation\\n");
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Person allocated, status: %d\\n", p1->status);
        freePerson(p1);
    }
    
    printf("\\nTest Case 2: Multiple allocations\\n");
    Person* p2 = allocatePerson();
    Person* p3 = allocatePerson();
    if (p2 != NULL && p3 != NULL) {
        printf("Multiple persons allocated successfully\\n");
        freePerson(p2);
        freePerson(p3);
    }
    
    printf("\\nTest Case 3: Verify status initialization\\n");
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        printf("Status is zero: %d\\n", p4->status == 0);
        freePerson(p4);
    }
    
    printf("\\nTest Case 4: Modify status\\n");
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        p5->status = 1;
        printf("Modified status: %d\\n", p5->status);
        freePerson(p5);
    }
    
    printf("\\nTest Case 5: Null check\\n");
    Person* p6 = allocatePerson();
    printf("Allocation result: %s\\n", p6 != NULL ? "Success" : "Failed");
    freePerson(p6);
    
    return 0;
}
