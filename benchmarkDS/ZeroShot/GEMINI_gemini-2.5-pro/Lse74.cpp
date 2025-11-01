#include <iostream>
#include <new> // Required for std::bad_alloc
#include <memory> // Required for std::unique_ptr

class Person {
private:
    int status;
    int id;

public:
    // Constructor initializes the object's state.
    // This is the secure way to ensure fields are always initialized.
    Person(int person_id) : status(0), id(person_id) {
        // The member initializer list ': status(0)' sets the status field.
    }

    void printDetails() const {
        std::cout << "  Successfully created Person with ID: " << id << std::endl;
        std::cout << "  Person status field is: " << status << std::endl;
    }
};

/**
 * Allocates a new Person object using a smart pointer for safety.
 * It allocates memory and the constructor sets the status field to 0.
 * Using std::unique_ptr automates memory deallocation, preventing leaks.
 * @param id The identifier for the new person.
 * @return A std::unique_ptr to the new Person, or nullptr on allocation failure.
 */
std::unique_ptr<Person> createPerson(int id) {
    try {
        // `new` allocates memory and calls the constructor.
        return std::make_unique<Person>(id);
    } catch (const std::bad_alloc& e) {
        // `new` throws std::bad_alloc on failure. This is the C++ way to handle it.
        std::cerr << "Fatal: Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        
        // createPerson returns a unique_ptr, which manages the allocated memory.
        auto p = createPerson(i);

        if (p) { // Check if the pointer is not null
            p->printDetails();
            // No need to call 'delete p;'.
            // The memory is automatically freed when 'p' goes out of scope at the
            // end of the loop iteration, preventing memory leaks.
        } else {
            std::cout << "  Failed to create Person." << std::endl;
            break;
        }
    }
    return 0;
}