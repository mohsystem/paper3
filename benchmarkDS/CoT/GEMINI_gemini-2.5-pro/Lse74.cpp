#include <iostream>
#include <vector>
#include <new> // Required for std::bad_alloc

// Define the Person structure
struct Person {
    int status;
    int id;

    // Constructor to initialize members.
    // This ensures status is always set to 0 on creation.
    Person() : status(0), id(-1) {}
};

/**
 * Allocates a block of memory for a Person, initializes its status field to 0,
 * and returns a pointer to it.
 * 
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* createPerson() {
    try {
        // The 'new' operator allocates memory and calls the default constructor,
        // which initializes status to 0.
        Person* newPerson = new Person();
        return newPerson;
    } catch (const std::bad_alloc& e) {
        // 'new' throws a std::bad_alloc exception on memory allocation failure.
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    std::vector<Person*> people; // Use a vector to keep track of pointers to delete

    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ":" << std::endl;
        Person* p = createPerson();

        // Security: Always check if the pointer is not null before using it.
        if (p != nullptr) {
            people.push_back(p); // Store for later cleanup
            std::cout << "  Person created successfully." << std::endl;
            std::cout << "  Initial status: " << p->status << std::endl;
        } else {
            std::cout << "  Failed to create person." << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    // Security: Clean up all allocated memory to prevent memory leaks.
    std::cout << "Cleaning up allocated memory..." << std::endl;
    for (Person* p : people) {
        delete p;
    }
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}