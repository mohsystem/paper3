#include <iostream>
#include <vector>
#include <new> // Required for std::nothrow

// A simple structure to represent a Person.
struct Person {
    int status;
    int id; // Added for testing purposes

    // Constructor to ensure members are always initialized.
    // This is an idiomatic C++ way to ensure object state is valid upon creation.
    Person(int person_id) : id(person_id) {
        // Explicitly set status to 0 as per the requirement.
        status = 0;
    }
};

/**
 * Allocates a block of memory for a new Person and initializes it.
 *
 * This function uses 'new (std::nothrow)', which is a secure way to request memory.
 * Instead of throwing a std::bad_alloc exception on failure, it returns a nullptr.
 * This allows for robust error checking similar to C's malloc.
 *
 * @param id The identifier for the new person.
 * @return A pointer to the newly allocated Person, or nullptr if allocation fails.
 */
Person* allocatePerson(int id) {
    // new (std::nothrow) prevents exceptions on allocation failure.
    // The constructor Person(id) is called, which sets status to 0.
    Person* p = new (std::nothrow) Person(id);
    return p; // Will be nullptr if allocation failed
}

int main() {
    // Use a vector to manage the pointers for easy cleanup.
    std::vector<Person*> people;
    std::cout << "Running 5 test cases..." << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        Person* p = allocatePerson(i);

        // Secure programming practice: always check the pointer returned by an
        // allocation function.
        if (p != nullptr) {
            people.push_back(p);
            std::cout << "Successfully allocated: Person{id=" << p->id << ", status=" << p->status << "}" << std::endl;
        } else {
            std::cerr << "Memory allocation failed for person with id " << i << std::endl;
            // Depending on the application, you might want to break or exit here.
        }
    }

    // Secure programming practice: free all allocated memory to prevent leaks.
    std::cout << "\nCleaning up allocated memory..." << std::endl;
    for (Person* p : people) {
        delete p;
    }
    // The vector itself is automatically destroyed when it goes out of scope.
    
    std::cout << "Program finished." << std::endl;
    return 0;
}