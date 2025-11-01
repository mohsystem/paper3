#include <iostream>
#include <new> // Required for std::nothrow

// Define the Person structure
struct Person {
    int status;
};

/**
 * Allocates a block of memory for a Person, initializes status to 0,
 * and returns a pointer to it.
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // Use `new (std::nothrow)` to prevent an exception on allocation failure.
    // It returns nullptr instead, which is safer to check.
    Person* p = new (std::nothrow) Person;
    if (p != nullptr) {
        // Allocation was successful, now initialize the members.
        p->status = 0;
    }
    return p;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << std::endl;
        Person* p = allocatePerson();

        // Always check if the pointer is valid before using it.
        if (p != nullptr) {
            std::cout << "Person allocated successfully at address: " << p << std::endl;
            std::cout << "Person status: " << p->status << std::endl;
            
            // Free the allocated memory to prevent memory leaks.
            delete p;
            // Set pointer to null to prevent use-after-free vulnerabilities.
            p = nullptr; 
            std::cout << "Memory freed." << std::endl;
        } else {
            std::cerr << "Person allocation failed." << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }
    return 0;
}