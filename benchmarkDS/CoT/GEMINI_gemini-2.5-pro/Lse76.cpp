#include <iostream>

// Represents the person structure
struct Person {
    int status;

    // Constructor to initialize status to 0 upon creation
    Person() : status(0) {}
};

/**
 * Allocates a block of memory for a Person object using 'new'.
 * The constructor is automatically called, setting status to 0.
 *
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // 'new (std::nothrow)' allocates memory and returns nullptr on failure
    // instead of throwing an exception, which is safer for C-style error checking.
    Person* p = new (std::nothrow) Person();
    return p; // If allocation fails, p will be nullptr
}

int main() {
    std::cout << "Running 5 test cases in C++:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        Person* p = allocatePerson();

        // Security: Always check if the pointer is null before using it.
        if (p != nullptr) {
            std::cout << "Person allocated successfully." << std::endl;
            std::cout << "Person status: " << p->status << std::endl;
            
            // Security: Free the allocated memory to prevent memory leaks.
            delete p;
            p = nullptr; // Best practice: set pointer to null after deleting.
        } else {
            std::cerr << "Failed to allocate memory for Person." << std::endl;
        }
    }
    return 0;
}