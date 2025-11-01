#include <iostream>
#include <string>
#include <vector>
#include <new> // Required for std::nothrow

// A simple struct to represent a person.
struct Person {
    int status;
    std::string name;
};

/**
 * @brief Securely allocates a new Person object on the heap.
 * The caller is responsible for deallocating the returned object using 'delete'.
 * @return Person* A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* createPerson() {
    // 'new (std::nothrow)' is a secure way to request memory allocation.
    // Instead of throwing a std::bad_alloc exception on failure, it returns nullptr.
    Person* person = new (std::nothrow) Person;

    // SECURITY: Always check if the allocation was successful.
    if (person != nullptr) {
        person->status = 0;
        // std::string handles its own memory securely, preventing buffer overflows.
        person->name = "John";
    }
    return person;
}

int main() {
    std::cout << "--- Running CPP Test Cases ---" << std::endl;
    std::vector<Person*> people; // Use a vector to manage pointers for cleanup

    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ":" << std::endl;
        Person* p = createPerson();

        if (p != nullptr) {
            std::cout << "  Created: Person[name=" << p->name << ", status=" << p->status << "]" << std::endl;
            std::cout << "  Memory Address: " << p << std::endl;
            people.push_back(p);
        } else {
            std::cerr << "  ERROR: Memory allocation failed!" << std::endl;
        }
    }

    std::cout << "\n--- Cleaning up allocated memory ---" << std::endl;
    for (Person* p : people) {
        // SECURITY: Deallocate the memory for each created object to prevent memory leaks.
        delete p;
    }
    // After deleting, the pointers in 'people' are dangling, but the vector
    // itself will be destroyed at the end of main's scope, so we don't need to clear it.
    std::cout << "--- CPP Test Cases Complete ---" << std::endl;

    return 0;
}