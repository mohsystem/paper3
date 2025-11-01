
#include <iostream>
#include <cstdlib>
#include <cstring>

struct Person {
    char name[50];
    int age;
    double height;
};

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
    std::cout << "Test 1: " << p1->name << ", " << p1->age << ", " << p1->height << std::endl;
    free(p1);
    
    // Test case 2
    Person* p2 = allocatePerson();
    strcpy(p2->name, "Alice");
    p2->age = 30;
    p2->height = 5.5;
    std::cout << "Test 2: " << p2->name << ", " << p2->age << ", " << p2->height << std::endl;
    free(p2);
    
    // Test case 3
    Person* p3 = allocatePerson();
    strcpy(p3->name, "Bob");
    p3->age = 35;
    p3->height = 6.1;
    std::cout << "Test 3: " << p3->name << ", " << p3->age << ", " << p3->height << std::endl;
    free(p3);
    
    // Test case 4
    Person* p4 = allocatePerson();
    strcpy(p4->name, "Carol");
    p4->age = 28;
    p4->height = 5.7;
    std::cout << "Test 4: " << p4->name << ", " << p4->age << ", " << p4->height << std::endl;
    free(p4);
    
    // Test case 5
    Person* p5 = allocatePerson();
    strcpy(p5->name, "David");
    p5->age = 40;
    p5->height = 5.8;
    std::cout << "Test 5: " << p5->name << ", " << p5->age << ", " << p5->height << std::endl;
    free(p5);
    
    return 0;
}
