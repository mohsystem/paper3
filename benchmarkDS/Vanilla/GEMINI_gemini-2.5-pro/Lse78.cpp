#include <iostream>

struct Person {
    int status;
    // You can add other person attributes here
    // std::string name;
    // int age;
};

/**
 * Allocates a new Person object on the heap and sets its status to 0.
 * @return A pointer to the newly allocated Person object.
 */
Person* allocatePerson() {
    Person* newPerson = new (std::nothrow) Person; // Use nothrow to prevent exceptions on failure
    if (newPerson != nullptr) {
        newPerson->status = 0;
    }
    return newPerson;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Test Case 1: Person allocated with status: " << p1->status << std::endl;
        delete p1; // Free the allocated memory
    } else {
        std::cerr << "Test Case 1: Memory allocation failed!" << std::endl;
    }

    // Test Case 2
    Person* p2 = allocatePerson();
    if (p2 != nullptr) {
        std::cout << "Test Case 2: Person allocated with status: " << p2->status << std::endl;
        delete p2;
    } else {
        std::cerr << "Test Case 2: Memory allocation failed!" << std::endl;
    }

    // Test Case 3
    Person* p3 = allocatePerson();
    if (p3 != nullptr) {
        std::cout << "Test Case 3: Person allocated with status: " << p3->status << std::endl;
        delete p3;
    } else {
        std::cerr << "Test Case 3: Memory allocation failed!" << std::endl;
    }

    // Test Case 4
    Person* p4 = allocatePerson();
    if (p4 != nullptr) {
        std::cout << "Test Case 4: Person allocated with status: " << p4->status << std::endl;
        delete p4;
    } else {
        std::cerr << "Test Case 4: Memory allocation failed!" << std::endl;
    }

    // Test Case 5
    Person* p5 = allocatePerson();
    if (p5 != nullptr) {
        std::cout << "Test Case 5: Person allocated with status: " << p5->status << std::endl;
        delete p5;
    } else {
        std::cerr << "Test Case 5: Memory allocation failed!" << std::endl;
    }

    return 0;
}