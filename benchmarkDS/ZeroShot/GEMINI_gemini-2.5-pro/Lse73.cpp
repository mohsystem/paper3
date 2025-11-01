#include <iostream>
#include <new> // Required for std::nothrow

class Person {
    // Members can be added here, e.g., std::string name; int age;
public:
    // A constructor is good practice to initialize members.
    Person() {
        // Initialization logic here.
    }
};

/**
 * Securely allocates a block of memory for a Person object.
 * Uses `new (std::nothrow)` which returns nullptr on allocation failure
 * instead of throwing a std::bad_alloc exception. This allows for
 * simple and explicit null-checking for error handling.
 *
 * @return A pointer to the newly allocated Person object, or nullptr on failure.
 */
Person* allocatePerson() {
    return new (std::nothrow) Person();
}

/**
 * Securely deallocates memory for a Person object.
 *
 * @param p A pointer to the Person object to be deallocated.
 */
void deallocatePerson(Person* p) {
    // `delete` on a nullptr is a safe no-op, so an explicit check is not
    // strictly required, but it doesn't hurt and can improve clarity.
    delete p;
}

int main() {
    std::cout << "Running C++ Test Cases..." << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;

        // 1. Allocate memory for a Person object.
        Person* p = allocatePerson();

        // 2. Securely check if the allocation was successful.
        if (p != nullptr) {
            std::cout << "Successfully allocated a Person object at address: " << p << std::endl;
            // The object can now be used.
        } else {
            std::cerr << "Failed to allocate memory for Person object." << std::endl;
        }

        // 3. Deallocate the memory to prevent memory leaks.
        deallocatePerson(p);
        if (p != nullptr) {
           std::cout << "Deallocated memory for Person object." << std::endl;
        }
        p = nullptr; // Good practice to nullify pointer after deallocation.
    }

    return 0;
}