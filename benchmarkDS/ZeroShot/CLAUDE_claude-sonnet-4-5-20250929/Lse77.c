
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
    char* name;
} Person;

Person* createPerson(const char* name) {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person == NULL) {
        return NULL;
    }
    
    person->status = 0;
    
    if (name != NULL) {
        size_t len = strlen(name);
        person->name = (char*)malloc(len + 1);
        if (person->name != NULL) {
            strcpy(person->name, name);
        }
    } else {
        person->name = NULL;
    }
    
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

void printPerson(const Person* person) {
    if (person != NULL) {
        printf("Person{status=%d, name='%s'}\\n", 
               person->status, 
               person->name ? person->name : "");
    }
}

int main() {
    // Test case 1: Create person with name "John"
    Person* person1 = createPerson("John");
    printf("Test 1: ");
    printPerson(person1);
    freePerson(person1);
    
    // Test case 2: Create person with name "Alice"
    Person* person2 = createPerson("Alice");
    printf("Test 2: ");
    printPerson(person2);
    freePerson(person2);
    
    // Test case 3: Create person with name "Bob"
    Person* person3 = createPerson("Bob");
    printf("Test 3: ");
    printPerson(person3);
    freePerson(person3);
    
    // Test case 4: Create person with empty name
    Person* person4 = createPerson("");
    printf("Test 4: ");
    printPerson(person4);
    freePerson(person4);
    
    // Test case 5: Create person with NULL name (secure handling)
    Person* person5 = createPerson(NULL);
    printf("Test 5: ");
    printPerson(person5);
    freePerson(person5);
    
    return 0;
}
