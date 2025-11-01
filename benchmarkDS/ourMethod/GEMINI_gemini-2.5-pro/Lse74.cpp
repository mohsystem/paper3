#include <iostream>
#include <new> // Required for std::nothrow

struct Person {
    int status;
    int id;

    // Constructor to initialize fields
    Person(int person_id) : id(person_id) {
        // The status is set to 0 upon creation.
        this->status = 0;
    }
};

/**
 * Allocates a block of memory for a Person object, initializes it,
 * and sets the status field to 0.
 *
 * @param id The identifier for the new person.
 * @return A pointer to the newly allocated Person, or nullptr on failure.
 */
Person* allocatePerson(int id) {
    // Use `new (std::nothrow)` which returns nullptr on allocation failure
    // instead of throwing an exception. This is a safer pattern for C-style
    // error handling.
    Person* p = new (std::nothrow) Person(id);
    return p;
}

void run_test(int test_num, int id) {
    Person* p = allocatePerson(id);
    if (p != nullptr && p->status == 0) {
        std::cout << "Test Case " << test_num << " PASSED: Person{id=" << p->id << ", status=" << p->status << "}" << std::endl;
    } else {
        std::cerr << "Test Case " << test_num << " FAILED: Allocation failed or status not 0." << std::endl;
    }
    // Deallocate the memory to prevent memory leaks.
    delete p;
}

int main() {
    std::cout << "Running C++ test cases..." << std::endl;
    
    // Test cases
    run_test(1, 301);
    run_test(2, 302);
    run_test(3, 303);
    run_test(4, 304);
    run_test(5, 305);

    return 0;
}