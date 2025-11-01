
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[50];
    int age;
} Person;

Person* allocatePerson() {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    memset(p, 0, sizeof(Person));
    return p;
}

Person* allocatePersonWithData(const char* name, int age) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    memset(p, 0, sizeof(Person));
    if (name != NULL) {
        strncpy(p->name, name, sizeof(p->name) - 1);
        p->name[sizeof(p->name) - 1] = '\\0';
    }
    p->age = age;
    return p;
}

void deallocatePerson(Person* p) {
    if (p != NULL) {
        free(p);
    }
}

int main() {
    // Test case 1: Basic allocation
    Person* p1 = allocatePerson();
    if (p1 != NULL) {
        strncpy(p1->name, "Alice", sizeof(p1->name) - 1);
        p1->age = 25;
        printf("Test 1: Person{name='%s', age=%d}\\n", p1->name, p1->age);
        deallocatePerson(p1);
    }
    
    // Test case 2: Allocation with parameters
    Person* p2 = allocatePersonWithData("Bob", 30);
    if (p2 != NULL) {
        printf("Test 2: Person{name='%s', age=%d}\\n", p2->name, p2->age);
        deallocatePerson(p2);
    }
    
    // Test case 3: Multiple allocations
    Person* p3 = allocatePersonWithData("Charlie", 35);
    if (p3 != NULL) {
        printf("Test 3: Person{name='%s', age=%d}\\n", p3->name, p3->age);
        deallocatePerson(p3);
    }
    
    // Test case 4: Null validation
    Person* p4 = allocatePersonWithData("Diana", 28);
    if (p4 != NULL) {
        printf("Test 4: Person{name='%s', age=%d}\\n", p4->name, p4->age);
        deallocatePerson(p4);
    } else {
        printf("Test 4: Allocation failed\\n");
    }
    
    // Test case 5: Zero age
    Person* p5 = allocatePersonWithData("Eve", 0);
    if (p5 != NULL) {
        printf("Test 5: Person{name='%s', age=%d}\\n", p5->name, p5->age);
        deallocatePerson(p5);
    }
    
    return 0;
}
