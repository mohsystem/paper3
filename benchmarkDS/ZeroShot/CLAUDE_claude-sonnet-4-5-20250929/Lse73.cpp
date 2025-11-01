
#include <iostream>
#include <cstring>
#include <cstdlib>

struct Person {
    char name[100];
    int age;
    double height;
};

Person* allocatePerson(const char* name, int age, double height) {
    Person* p = (Person*)malloc(sizeof(Person));
    if (p != nullptr) {
        strncpy(p->name, name, sizeof(p->name) - 1);
        p->name[sizeof(p->name) - 1] = '\\0';
        p->age = age;
        p->height = height;
    }
    return p;
}

void printPerson(const Person* p) {
    if (p != nullptr) {
        std::cout << "Person{name='" << p->name << "', age=" << p->age 
                  << ", height=" << p->height << "}" << std::endl;
    }
}

int main() {
    // Test case 1
    Person* p1 = allocatePerson("John Doe", 30, 5.9);
    std::cout << "Test 1: ";
    printPerson(p1);
    
    // Test case 2
    Person* p2 = allocatePerson("Jane Smith", 25, 5.5);
    std::cout << "Test 2: ";
    printPerson(p2);
    
    // Test case 3
    Person* p3 = allocatePerson("Bob Johnson", 45, 6.1);
    std::cout << "Test 3: ";
    printPerson(p3);
    
    // Test case 4
    Person* p4 = allocatePerson("Alice Williams", 35, 5.7);
    std::cout << "Test 4: ";
    printPerson(p4);
    
    // Test case 5
    Person* p5 = allocatePerson("Charlie Brown", 28, 5.8);
    std::cout << "Test 5: ";
    printPerson(p5);
    
    // Free allocated memory
    free(p1);
    free(p2);
    free(p3);
    free(p4);
    free(p5);
    
    return 0;
}
