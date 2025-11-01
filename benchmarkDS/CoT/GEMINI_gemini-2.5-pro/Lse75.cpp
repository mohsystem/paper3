#include <iostream>
#include <vector>
#include <new> // Required for std::bad_alloc

struct Person {
    int status;
    int id;
    static int counter;

    // Constructor to initialize members when a new object is created.
    Person() {
        // Set status to 0 as required
        status = 0;
        id = ++counter;
    }
};

// Initialize the static member variable.
int Person::counter = 0;

/**
 * Allocates a block of memory for a Person, initializes it by calling
 * its constructor, and returns a pointer to the block.
 *
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // The 'new' operator allocates memory and calls the constructor.
    // The constructor sets the status to 0.
    // In modern C++, returning a std::unique_ptr would be safer practice
    // to prevent memory leaks automatically.
    try {
        return new Person();
    } catch (const std::bad_alloc& e) {
        // 'new' throws a std::bad_alloc exception on failure.
        // We catch it and return nullptr to indicate failure.
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    // Use a vector to safely store the pointers for later cleanup.
    std::vector<Person*> people;

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        Person* p = allocatePerson();

        // Always check if the pointer is valid after allocation.
        if (p != nullptr) {
            people.push_back(p); // Store the valid pointer for cleanup.
            std::cout << "  Allocated: Person(id=" << p->id << ", status=" << p->status << ")" << std::endl;
            std::cout << "  Person status is: " << p->status << std::endl;
        } else {
            std::cout << "  Memory allocation failed." << std::endl;
            break; // Stop if we can't allocate more memory.
        }
    }

    // IMPORTANT: In C++, manually allocated memory must be manually deallocated.
    // Failure to do so results in a memory leak.
    std::cout << "\n--- Cleaning up allocated memory ---" << std::endl;
    for (Person* p : people) {
        std::cout << "  Deleting Person with id " << p->id << std::endl;
        delete p;
    }
    // The vector itself will be cleaned up automatically when it goes out of scope.

    return 0;
}