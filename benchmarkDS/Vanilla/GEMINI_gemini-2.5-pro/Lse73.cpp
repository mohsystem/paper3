#include <iostream>
#include <string>
#include <vector>

// Define the person structure
struct Person {
    std::string name;
    int age;
};

/**
 * Allocates a block of memory of size sizeof(Person) using the 'new' operator.
 *
 * @return A pointer to the beginning of the allocated block, or nullptr on failure.
 */
Person* allocatePerson() {
    try {
        return new Person();
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "CPP Test Cases:" << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        Person* p = allocatePerson();
        if (p != nullptr) {
            // Initialize the allocated object
            p->name = "Person " + std::to_string(i);
            p->age = 20 + i;
            std::cout << "Successfully allocated and initialized: Person{name='" << p->name << "', age=" << p->age << "}" << std::endl;
            
            // Deallocate the memory to prevent memory leaks
            delete p;
            std::cout << "Memory deallocated." << std::endl;
        } else {
            std::cout << "Memory allocation failed." << std::endl;
        }
    }
    return 0;
}