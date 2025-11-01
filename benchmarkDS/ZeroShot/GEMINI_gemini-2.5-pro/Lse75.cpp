#include <iostream>
#include <vector>
#include <memory> // For std::unique_ptr

// A struct to represent a person.
// Using a constructor is the C++ way to ensure secure initialization.
struct Person {
    int status;
    int id;

    // Constructor to initialize members.
    Person(int person_id) : status(0), id(person_id) {
        // The member initializer list sets status to 0.
    }
};

/**
 * Creates a new person and returns a smart pointer to it.
 * Using std::unique_ptr ensures automatic memory deallocation (RAII),
 * preventing memory leaks.
 *
 * @param id The identifier for the new person.
 * @return A unique_ptr to the newly allocated Person, or nullptr on failure.
 */
std::unique_ptr<Person> createPerson(int id) {
    // std::make_unique is the modern, exception-safe way to allocate dynamic memory.
    // It allocates memory using 'new' and wraps it in a unique_ptr.
    // The constructor Person(id) is called, which sets status to 0.
    try {
        return std::make_unique<Person>(id);
    } catch (const std::bad_alloc& e) {
        // 'new' throws std::bad_alloc on allocation failure.
        std::cerr << "Error: Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;
    std::vector<std::unique_ptr<Person>> people;

    // 5 Test Cases
    for (int i = 1; i <= 5; ++i) {
        auto p = createPerson(i * 10);
        if (p) {
            std::cout << "Test Case " << i << ": Created Person{id=" << p->id
                      << ", status=" << p->status << "}" << std::endl;
            people.push_back(std::move(p)); // Move ownership into the vector
        } else {
            std::cout << "Test Case " << i << ": Person creation failed." << std::endl;
        }
    }

    // Memory is automatically freed when the unique_ptr objects in the vector
    // go out of scope at the end of main. No need for manual 'delete'.
    std::cout << "Total people created: " << people.size() << std::endl;
    return 0;
}