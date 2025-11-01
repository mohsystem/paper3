#include <iostream>
#include <new> // Required for std::nothrow

// Define the Person structure
struct Person {
    int status;

    // Constructor to initialize status to 0
    Person() : status(0) {}
};

/**
 * Allocates a block of memory for a Person object and initializes it.
 * Using 'new (std::nothrow)' is a secure practice as it returns nullptr
 * on allocation failure instead of throwing an exception, allowing for
 * graceful error handling. The constructor ensures status is initialized.
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    return new (std::nothrow) Person;
}

int main() {
    std::cout << "Running C++ test cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        Person* p = allocatePerson();

        // Security: Always check if the allocation was successful
        if (p) {
            std::cout << "  Person allocated successfully." << std::endl;
            std::cout << "  Person status: " << p->status << std::endl;

            // Security: Free the allocated memory to prevent memory leaks
            delete p;
            p = nullptr; // Best practice to avoid dangling pointers
        } else {
            std::cerr << "  Person allocation failed." << std::endl;
        }
    }
    return 0;
}