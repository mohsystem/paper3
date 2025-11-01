
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[50];
    int age;
    double height;
} Person;

Person* allocatePerson() {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p != NULL) {
        strcpy(p->name, "");
        p->age = 0;
        p->height = 0.0;
    }
    return p;
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson();
    strcpy(p1->name, "John");
    p1->age = 25;
    p1->height = 5.9;
    printf("Test 1: %s, %d, %.1f\\n", p1->name, p1->age, p1->height);
    free(p1);
    
    // Test case 2
    Person* p2 = allocatePerson();
    strcpy(p2->name, "Alice");
    p2->age = 30;
    p2->height = 5.5;
    printf("Test 2: %s, %d, %.1f\\n", p2->name, p2->age, p2->height);
    free(p2);
    
    // Test case 3
    Person* p3 = allocatePerson();
    strcpy(p3->name, "Bob");
    p3->age = 35;
    p3->height = 6.1;
    printf("Test 3: %s, %d, %.1f\\n", p3->name, p3->age, p3->height);
    free(p3);
    
    // Test case 4
    Person* p4 = allocatePerson();
    strcpy(p4->name, "Carol");
    p4->age = 28;
    p4->height = 5.7;
    printf("Test 4: %s, %d, %.1f\\n", p4->name, p4->age, p4->height);
    free(p4);
    
    // Test case 5
    Person* p5 = allocatePerson();
    strcpy(p5->name, "David");
    p5->age = 40;
    p5->height = 5.8;
    printf("Test 5: %s, %d, %.1f\\n", p5->name, p5->age, p5->height);
    free(p5);
    
    return 0;
}
