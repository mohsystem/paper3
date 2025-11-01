
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
    printf("Test 1 - Person status: %d\\n", p1->status);
    
    // Test case 2
    Person* p2 = allocatePerson();
    printf("Test 2 - Person status: %d\\n", p2->status);
    
    // Test case 3
    Person* p3 = allocatePerson();
    printf("Test 3 - Person status: %d\\n", p3->status);
    
    // Test case 4
    Person* p4 = allocatePerson();
    printf("Test 4 - Person status: %d\\n", p4->status);
    
    // Test case 5
    Person* p5 = allocatePerson();
    printf("Test 5 - Person status: %d\\n", p5->status);
    
    // Free allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
