#include <iostream>
#include <new> // Required for std::nothrow

// Define the Person structure
struct Person {
    int status;
    int age;
};

/**
 * Allocates a block of memory for a Person, sets its status to 0,
 * and returns a pointer to the block.
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // Allocate memory using `new` with `nothrow` to prevent exceptions on failure.
    // It returns nullptr if allocation fails.
    Person* newPerson = new (std::nothrow) Person;

    if (newPerson != nullptr) {
        // If allocation was successful, set the status to 0.
        newPerson->status = 0;
        newPerson->age = -1; // Initialize other members as a good practice
    }
    
    return newPerson;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        Person* p = allocatePerson();
        if (p != nullptr) {
            std::cout << "Test Case " << i << ": Allocated Person at " << p << ". Status: " << p->status << std::endl;
            // It's crucial to deallocate the memory to prevent memory leaks.
            delete p;
        } else {
            std::cout << "Test Case " << i << ": Memory allocation failed." << std::endl;
        }
    }
    return 0;
}