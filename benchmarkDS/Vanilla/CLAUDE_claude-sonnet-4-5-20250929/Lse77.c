
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char* name;
} Person;

Person* createPerson(const char* name) {
    Person* person = (Person*)malloc(sizeof(Person));
    person->status = 0;
    person->name = (char*)malloc(strlen(name) + 1);
    strcpy(person->name, name);
    return person;
}

void freePerson(Person* person) {
    if (person != NULL) {
        if (person->name != NULL) {
            free(person->name);
        }
        free(person);
    }
}

int main() {
    // Test case 1
    Person* person1 = createPerson("John");
    printf("Test 1 - Name: %s, Status: %d\\n", person1->name, person1->status);
    freePerson(person1);
    
    // Test case 2
    Person* person2 = createPerson("Alice");
    printf("Test 2 - Name: %s, Status: %d\\n", person2->name, person2->status);
    freePerson(person2);
    
    // Test case 3
    Person* person3 = createPerson("Bob");
    printf("Test 3 - Name: %s, Status: %d\\n", person3->name, person3->status);
    freePerson(person3);
    
    // Test case 4
    Person* person4 = createPerson("Emma");
    printf("Test 4 - Name: %s, Status: %d\\n", person4->name, person4->status);
    freePerson(person4);
    
    // Test case 5
    Person* person5 = createPerson("David");
    printf("Test 5 - Name: %s, Status: %d\\n", person5->name, person5->status);
    freePerson(person5);
    
    return 0;
}
