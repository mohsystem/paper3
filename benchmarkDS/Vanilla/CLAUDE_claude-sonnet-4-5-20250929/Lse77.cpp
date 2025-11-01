
#include <iostream>
#include <cstring>
#include <cstdlib>

struct Person {
    int status;
    char* name;
};

Person* createPerson(const char* name) {
    Person* person = (Person*)malloc(sizeof(Person));
    person->status = 0;
    person->name = (char*)malloc(strlen(name) + 1);
    strcpy(person->name, name);
    return person;
}

void freePerson(Person* person) {
    if (person != nullptr) {
        if (person->name != nullptr) {
            free(person->name);
        }
        free(person);
    }
}

int main() {
    // Test case 1
    Person* person1 = createPerson("John");
    std::cout << "Test 1 - Name: " << person1->name << ", Status: " << person1->status << std::endl;
    freePerson(person1);
    
    // Test case 2
    Person* person2 = createPerson("Alice");
    std::cout << "Test 2 - Name: " << person2->name << ", Status: " << person2->status << std::endl;
    freePerson(person2);
    
    // Test case 3
    Person* person3 = createPerson("Bob");
    std::cout << "Test 3 - Name: " << person3->name << ", Status: " << person3->status << std::endl;
    freePerson(person3);
    
    // Test case 4
    Person* person4 = createPerson("Emma");
    std::cout << "Test 4 - Name: " << person4->name << ", Status: " << person4->status << std::endl;
    freePerson(person4);
    
    // Test case 5
    Person* person5 = createPerson("David");
    std::cout << "Test 5 - Name: " << person5->name << ", Status: " << person5->status << std::endl;
    freePerson(person5);
    
    return 0;
}
