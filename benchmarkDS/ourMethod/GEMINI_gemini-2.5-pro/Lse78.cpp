#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <new> // For std::bad_alloc

class Person {
public:
    // Constructor initializes the person object and sets status to 0.
    explicit Person(const std::string& name) : name_(name), status_(0) {}

    void print() const {
        std::cout << "Person{name='" << name_ << "', status=" << status_ << "}" << std::endl;
    }

private:
    std::string name_;
    int status_;
};

/**
 * @brief Allocates a new person and sets the status to 0 using modern C++ practices.
 *
 * This factory function uses std::unique_ptr for automatic memory management,
 * preventing memory leaks.
 *
 * @param name The name of the person.
 * @return A std::unique_ptr to the new Person object, or nullptr if allocation fails or name is invalid.
 */
std::unique_ptr<Person> createPerson(const std::string& name) {
    if (name.empty()) {
        std::cerr << "Error: Name cannot be empty." << std::endl;
        return nullptr;
    }
    // Using smart pointers is the recommended modern C++ approach for memory management.
    // std::make_unique handles `new` and wraps it in a unique_ptr.
    // If `new` fails, it throws std::bad_alloc, which we catch for graceful failure.
    try {
        return std::make_unique<Person>(name);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    std::vector<std::unique_ptr<Person>> people;
    people.reserve(5); // Pre-allocate space for 5 people

    // Test Case 1
    if (auto person1 = createPerson("Alice")) {
        std::cout << "Test Case 1: Created ";
        person1->print();
        people.push_back(std::move(person1));
    }

    // Test Case 2
    if (auto person2 = createPerson("Bob")) {
        std::cout << "Test Case 2: Created ";
        person2->print();
        people.push_back(std::move(person2));
    }

    // Test Case 3
    if (auto person3 = createPerson("Charlie")) {
        std::cout << "Test Case 3: Created ";
        person3->print();
        people.push_back(std::move(person3));
    }

    // Test Case 4
    if (auto person4 = createPerson("Diana")) {
        std::cout << "Test Case 4: Created ";
        person4->print();
        people.push_back(std::move(person4));
    }

    // Test Case 5: Invalid input
    if (auto person5 = createPerson("")) {
        std::cout << "Test Case 5: Created ";
        person5->print();
        people.push_back(std::move(person5));
    } else {
        std::cout << "Test Case 5: Failed to create person as expected for empty name." << std::endl;
    }

    // Memory for all created Person objects is automatically deallocated
    // when the 'people' vector goes out of scope, thanks to std::unique_ptr's RAII nature.
    std::cout << "\n--- Test Cases Finished ---" << std::endl;

    return 0;
}