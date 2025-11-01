#include <iostream>
#include <vector>

struct Person {
    int status;
};

/**
 * Allocates a block of memory of size sizeof(Person) on the heap
 * and returns a pointer to the beginning of the block.
 * The new person's status field is set to 0.
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson() {
    // 'new' allocates memory and calls the constructor. For a POD struct,
    // it just allocates memory. We can use () to value-initialize members.
    Person* p = new (std::nothrow) Person(); // std::nothrow prevents exceptions
    if (p) { // Check if allocation was successful
        p->status = 0; // Set status field to 0
    }
    return p;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    std::vector<Person*> people; // To store pointers for later deletion

    for (int i = 1; i <= 5; ++i) {
        Person* person = allocatePerson();
        std::cout << "Test Case " << i << ":" << std::endl;
        if (person) {
            people.push_back(person); // Store for cleanup
            std::cout << "  Person allocated at: " << person << std::endl;
            std::cout << "  Person status: " << person->status << std::endl;
        } else {
            std::cout << "  Allocation failed." << std::endl;
        }
    }

    // Clean up allocated memory to prevent memory leaks
    std::cout << "\nCleaning up memory..." << std::endl;
    for (Person* p : people) {
        delete p;
    }
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}