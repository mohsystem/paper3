
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[50];
    int age;
    int status;
} Person;

Person* allocatePerson(const char* name, int age) {
    Person* person = (Person*)malloc(sizeof(Person));
    if (person != NULL) {
        strncpy(person->name, name, 49);
        person->name[49] = '\\0';
        person->age = age;
        person->status = 0;
    }
    return person;
}

void freePerson(Person* person) {
    if (person != NULL) {
        free(person);
    }
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson("Alice", 25);
    printf("Test 1: Person{name='%s', age=%d, status=%d}\\n", 
           p1->name, p1->age, p1->status);
    
    // Test case 2
    Person* p2 = allocatePerson("Bob", 30);
    printf("Test 2: Person{name='%s', age=%d, status=%d}\\n", 
           p2->name, p2->age, p2->status);
    
    // Test case 3
    Person* p3 = allocatePerson("Charlie", 35);
    printf("Test 3: Person{name='%s', age=%d, status=%d}\\n", 
           p3->name, p3->age, p3->status);
    
    // Test case 4
    Person* p4 = allocatePerson("David", 40);
    printf("Test 4: Person{name='%s', age=%d, status=%d}\\n", 
           p4->name, p4->age, p4->status);
    
    // Test case 5
    Person* p5 = allocatePerson("Eve", 28);
    printf("Test 5: Person{name='%s', age=%d, status=%d}\\n", 
           p5->name, p5->age, p5->status);
    
    // Free allocated memory
    freePerson(p1);
    freePerson(p2);
    freePerson(p3);
    freePerson(p4);
    freePerson(p5);
    
    return 0;
}
