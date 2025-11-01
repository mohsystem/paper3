#include <iostream>
#include <string>
#include <vector>

// Define a structure for a person.
struct Person {
    int status;
    std::string name;
};

/**
 * Allocates a block of memory for a new Person on the heap.
 * Initializes the Person's status to 0 and name to "John".
 * 
 * @return A pointer to the newly allocated Person object, or nullptr on failure.
 */
Person* createPerson() {
    // 'new' allocates memory on the heap and calls the constructor.
    // We handle potential allocation failure by catching std::bad_alloc.
    try {
        Person* p = new Person;
        p->status = 0;
        p->name = "John";
        return p;
    } catch (const std::bad_alloc& e) {
        // std::cerr is used for printing errors.
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        Person* p = createPerson();

        // Always check if the pointer is not null before using it.
        if (p != nullptr) {
            std::cout << "  Successfully created person at address " << p 
                      << ": Status=" << p->status << ", Name='" << p->name << "'" << std::endl;
            
            // It's crucial to deallocate the memory to prevent memory leaks.
            delete p;
            p = nullptr; // Best practice: avoid dangling pointers.
            std::cout << "  Memory freed." << std::endl;
        } else {
            std::cout << "  Failed to create person." << std::endl;
        }
    }
    return 0;
}