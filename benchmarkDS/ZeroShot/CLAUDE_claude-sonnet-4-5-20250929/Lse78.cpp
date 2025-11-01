
#include <iostream>
#include <string>
#include <cstring>

struct Person {
    char name[50];
    int age;
    int status;
};

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
    std::cout << "Test 1: Person{name='" << p1->name << "', age=" << p1->age 
              << ", status=" << p1->status << "}" << std::endl;
    
    // Test case 2
    Person* p2 = allocatePerson("Bob", 30);
    std::cout << "Test 2: Person{name='" << p2->name << "', age=" << p2->age 
              << ", status=" << p2->status << "}" << std::endl;
    
    // Test case 3
    Person* p3 = allocatePerson("Charlie", 35);
    std::cout << "Test 3: Person{name='" << p3->name << "', age=" << p3->age 
              << ", status=" << p3->status << "}" << std::endl;
    
    // Test case 4
    Person* p4 = allocatePerson("David", 40);
    std::cout << "Test 4: Person{name='" << p4->name << "', age=" << p4->age 
              << ", status=" << p4->status << "}" << std::endl;
    
    // Test case 5
    Person* p5 = allocatePerson("Eve", 28);
    std::cout << "Test 5: Person{name='" << p5->name << "', age=" << p5->age 
              << ", status=" << p5->status << "}" << std::endl;
    
    // Free allocated memory
    freePerson(p1);
    freePerson(p2);
    freePerson(p3);
    freePerson(p4);
    freePerson(p5);
    
    return 0;
}
