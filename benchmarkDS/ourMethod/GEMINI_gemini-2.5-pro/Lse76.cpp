#include <iostream>
#include <vector>
#include <new> // Required for std::nothrow

/**
 * A simple structure to represent a person.
 */
struct Person {
    int status;

    // Constructor to ensure status is initialized to 0 upon creation.
    Person() : status(0) {}
};

/**
 * Allocates a block of memory of size sizeof(Person) and returns a pointer
 * to the starting of the block. The new person's status is set to 0.
 *
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // Use `new (std::nothrow)` to prevent throwing an exception on allocation
    // failure. Instead, it returns nullptr, which allows for safe checking.
    Person* p = new (std::nothrow) Person();
    
    // The constructor Person() is called automatically, which sets status to 0.
    return p;
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    
    std::vector<Person*> allocated_persons;
    allocated_persons.reserve(5);

    // Test cases: Allocate 5 Person objects
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case #" << i << std::endl;
        Person* p = allocatePerson();

        if (p != nullptr) {
            std::cout << "  Allocation successful at address: " << p << std::endl;
            std::cout << "  Status is correctly set to: " << p->status << std::endl;
            allocated_persons.push_back(p);
        } else {
            std::cerr << "  Memory allocation failed." << std::endl;
            // Clean up any memory that was successfully allocated before the failure.
            for (Person* person_ptr : allocated_persons) {
                delete person_ptr;
            }
            return 1; // Exit with an error code
        }
        std::cout << std::endl;
    }

    // IMPORTANT: Free all dynamically allocated memory to prevent memory leaks.
    std::cout << "Cleaning up allocated memory..." << std::endl;
    for (Person* p : allocated_persons) {
        delete p;
    }
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}