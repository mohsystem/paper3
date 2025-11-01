#include <iostream>
#include <new> // Required for std::bad_alloc

// A simple struct to represent a person
struct Person {
    int age;
    float height;
};

/**
 * Allocates a block of memory for a Person struct on the heap.
 * @return A pointer to the newly allocated Person.
 */
Person* allocatePerson() {
    // 'new' allocates memory of size sizeof(Person) and calls the default constructor.
    // It will throw std::bad_alloc on failure, which is the standard C++ way.
    return new Person;
}

int main() {
    std::cout << "--- CPP Test Cases ---" << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ":" << std::endl;
        Person* p = nullptr;
        
        try {
            // 1. Allocate memory for a Person struct
            p = allocatePerson();

            // 2. Initialize the struct's members
            p->age = 20 + i;
            p->height = 170.5f + i;

            // 3. Print the struct's state
            std::cout << "  Allocated and initialized: Person{age=" << p->age 
                      << ", height=" << p->height << "}" << std::endl;

        } catch (const std::bad_alloc& e) {
            std::cerr << "  Memory allocation failed: " << e.what() << std::endl;
        }

        // 4. Deallocate the memory to prevent memory leaks
        // This must be done for every successful allocation.
        if (p != nullptr) {
            delete p;
            p = nullptr; // Good practice to set pointer to null after deletion to avoid dangling pointers
            std::cout << "  Memory deallocated." << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}