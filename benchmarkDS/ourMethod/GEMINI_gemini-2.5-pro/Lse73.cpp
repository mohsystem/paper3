#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <new>
#include <stdexcept>

// A simple struct representing a person.
struct Person {
    std::string name;
    int age;

    Person(const std::string& n, int a) : name(n), age(a) {
        if (n.empty()) {
            throw std::invalid_argument("Name cannot be empty.");
        }
        if (a < 0) {
            throw std::invalid_argument("Age cannot be negative.");
        }
    }
};

/**
 * @brief Allocates memory for a Person object and returns a smart pointer.
 *
 * This function uses std::make_unique for safe, exception-safe memory allocation.
 * It allocates a block of memory of size sizeof(Person) on the heap.
 * The returned std::unique_ptr manages the lifetime of the object,
 * automatically freeing the memory when it goes out of scope (RAII).
 * 
 * @param name The name of the person.
 * @param age The age of the person.
 * @return A std::unique_ptr<Person> owning the newly created object.
 *         On allocation failure, std::make_unique throws std::bad_alloc.
 *         On invalid arguments, the constructor throws std::invalid_argument.
 */
std::unique_ptr<Person> allocatePerson(const std::string& name, int age) {
    // std::make_unique is the preferred way to create unique_ptrs.
    // It is exception-safe and avoids raw `new`.
    return std::make_unique<Person>(name, age);
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    // Using a vector of unique_ptr to manage the lifetime of all allocated persons.
    std::vector<std::unique_ptr<Person>> people;
    
    try {
        // Test Case 1
        people.push_back(allocatePerson("Alice", 30));
        std::cout << "Test Case 1: Allocated Person{name='" << people.back()->name << "', age=" << people.back()->age << "}" << std::endl;

        // Test Case 2
        people.push_back(allocatePerson("Bob", 25));
        std::cout << "Test Case 2: Allocated Person{name='" << people.back()->name << "', age=" << people.back()->age << "}" << std::endl;
        
        // Test Case 3
        people.push_back(allocatePerson("Charlie", 35));
        std::cout << "Test Case 3: Allocated Person{name='" << people.back()->name << "', age=" << people.back()->age << "}" << std::endl;
        
        // Test Case 4
        people.push_back(allocatePerson("Diana", 28));
        std::cout << "Test Case 4: Allocated Person{name='" << people.back()->name << "', age=" << people.back()->age << "}" << std::endl;
        
        // Test Case 5
        people.push_back(allocatePerson("Eve", 40));
        std::cout << "Test Case 5: Allocated Person{name='" << people.back()->name << "', age=" << people.back()->age << "}" << std::endl;

    } catch (const std::bad_alloc& e) {
        // Handle memory allocation failure.
        std::cerr << "FATAL: Memory allocation failed: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Total people allocated: " << people.size() << std::endl;
    std::cout << "Memory will be automatically deallocated when 'people' vector goes out of scope." << std::endl;
    std::cout << "--- C++ Test Cases Complete ---" << std::endl;
    // Destructors for all Person objects held by unique_ptrs in the vector
    // will be called here, freeing the memory.
    return 0;
}