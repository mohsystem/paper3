#include <iostream>
#include <string>
#include <new> // Required for std::bad_alloc

// A struct to represent a Person
struct Person {
    int status;
    std::string name;
};

/**
 * @brief Allocates a block of memory for a Person and initializes it.
 * 
 * Uses 'new' to allocate memory on the heap. This is safer than malloc
 * as it calls the constructor for objects like std::string.
 * 
 * @return A pointer to the newly created Person, or nullptr on failure.
 */
Person* createPerson() {
    try {
        Person* p = new Person();
        p->status = 0;
        p->name = "John";
        return p;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

/**
 * @brief Frees the memory allocated for a Person.
 * 
 * @param p A pointer to the Person to be deallocated.
 */
void destroyPerson(Person* p) {
    delete p;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        Person* person = createPerson();
        
        if (person) {
            std::cout << "  Person created successfully." << std::endl;
            std::cout << "  Name: " << person->name << ", Status: " << person->status << std::endl;
            destroyPerson(person); // Deallocate the memory to prevent leaks
            std::cout << "  Person destroyed successfully." << std::endl;
        } else {
            std::cout << "  Failed to create person." << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}