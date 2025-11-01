
// C++: Use smart pointers for automatic memory management and RAII
// This prevents memory leaks and use-after-free vulnerabilities

#include <iostream>
#include <memory>
#include <vector>

// Person class with status member
class Person {
private:
    int status;

public:
    // Constructor initializes status to 0 (secure default)
    Person() : status(0) {}
    
    // Prevent accidental copying (optional, for safety)
    Person(const Person&) = default;
    Person& operator=(const Person&) = default;
    
    int getStatus() const { return status; }
    void setStatus(int newStatus) { status = newStatus; }
    
    friend std::ostream& operator<<(std::ostream& os, const Person& p) {
        os << "Person{status=" << p.status << "}";
        return os;
    }
};

// Allocates a new Person using unique_ptr for automatic memory management
// Returns ownership of the allocated Person to the caller
std::unique_ptr<Person> allocatePerson() {
    // Use make_unique for exception safety and automatic cleanup
    // If allocation fails, std::bad_alloc is thrown, no leak occurs
    std::unique_ptr<Person> newPerson = std::make_unique<Person>();
    // Status is already initialized to 0 by constructor
    return newPerson;
}

int main() {
    // Test case 1: Basic allocation
    std::unique_ptr<Person> p1 = allocatePerson();
    std::cout << "Test 1: " << *p1 << std::endl;
    
    // Test case 2: Multiple allocations
    std::unique_ptr<Person> p2 = allocatePerson();
    std::unique_ptr<Person> p3 = allocatePerson();
    std::cout << "Test 2: " << *p2 << ", " << *p3 << std::endl;
    
    // Test case 3: Verify initial status
    std::unique_ptr<Person> p4 = allocatePerson();
    std::cout << "Test 3: Status is " << p4->getStatus() << std::endl;
    
    // Test case 4: Modify status after allocation
    std::unique_ptr<Person> p5 = allocatePerson();
    p5->setStatus(1);
    std::cout << "Test 4: Modified status to " << p5->getStatus() << std::endl;
    
    // Test case 5: Vector of persons (using unique_ptr)
    std::vector<std::unique_ptr<Person>> people;
    for (int i = 0; i < 3; ++i) {
        people.push_back(allocatePerson());
    }
    std::cout << "Test 5: Allocated " << people.size() << " persons" << std::endl;
    for (size_t i = 0; i < people.size(); ++i) {
        std::cout << "  Person " << i << ": " << *people[i] << std::endl;
    }
    
    // All memory automatically freed when unique_ptrs go out of scope
    return 0;
}
