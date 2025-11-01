
#include <iostream>
#include <string>
#include <cstring>

class Person {
public:
    int status;
    char* name;
    
    Person() : status(0), name(nullptr) {}
    
    ~Person() {
        if (name != nullptr) {
            delete[] name;
        }
    }
    
    void setName(const char* newName) {
        if (name != nullptr) {
            delete[] name;
        }
        if (newName != nullptr) {
            size_t len = strlen(newName);
            name = new char[len + 1];
            strcpy(name, newName);
        } else {
            name = nullptr;
        }
    }
    
    void print() const {
        std::cout << "Person{status=" << status << ", name='";
        if (name != nullptr) {
            std::cout << name;
        }
        std::cout << "'}" << std::endl;
    }
};

Person* createPerson(const char* name) {
    Person* person = new Person();
    person->status = 0;
    person->setName(name);
    return person;
}

int main() {
    // Test case 1: Create person with name "John"
    Person* person1 = createPerson("John");
    std::cout << "Test 1: ";
    person1->print();
    delete person1;
    
    // Test case 2: Create person with name "Alice"
    Person* person2 = createPerson("Alice");
    std::cout << "Test 2: ";
    person2->print();
    delete person2;
    
    // Test case 3: Create person with name "Bob"
    Person* person3 = createPerson("Bob");
    std::cout << "Test 3: ";
    person3->print();
    delete person3;
    
    // Test case 4: Create person with empty name
    Person* person4 = createPerson("");
    std::cout << "Test 4: ";
    person4->print();
    delete person4;
    
    // Test case 5: Create person with NULL name (secure handling)
    Person* person5 = createPerson(NULL);
    std::cout << "Test 5: ";
    person5->print();
    delete person5;
    
    return 0;
}
