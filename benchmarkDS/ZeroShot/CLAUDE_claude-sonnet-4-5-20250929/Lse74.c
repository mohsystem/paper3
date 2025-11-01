
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int status;
} Person;

Person* allocatePerson() {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        person->status = 0;
    }
    return person;
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        printf("Test 1 - Status: %d\\n", p1->status);
        free(p1);
    }
    
    // Test case 2
    Person* p2 = allocatePerson();
    if (p2 != NULL) {
        printf("Test 2 - Status: %d\\n", p2->status);
        free(p2);
    }
    
    // Test case 3
    Person* p3 = allocatePerson();
    if (p3 != NULL) {
        printf("Test 3 - Status: %d\\n", p3->status);
        free(p3);
    }
    
    // Test case 4
    Person* p4 = allocatePerson();
    if (p4 != NULL) {
        printf("Test 4 - Status: %d\\n", p4->status);
        free(p4);
    }
    
    // Test case 5
    Person* p5 = allocatePerson();
    if (p5 != NULL) {
        printf("Test 5 - Status: %d\\n", p5->status);
        free(p5);
    }
    
    return 0;
}
