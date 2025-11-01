
#include <iostream>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <limits>

// Person structure with validated fields
struct Person {
    char* name;
    int age;
    char* address;
    
    // Constructor with validation
    Person() : name(nullptr), age(0), address(nullptr) {
        // Allocate with bounds checking
        name = new(std::nothrow) char[101]; // 100 chars + null terminator
        if (!name) {
            throw std::bad_alloc();
        }
        name[0] = '\\0';
        
        address = new(std::nothrow) char[201]; // 200 chars + null terminator
        if (!address) {
            delete[] name;
            name = nullptr;
            throw std::bad_alloc();
        }
        address[0] = '\\0';
    }
    
    // Destructor to prevent memory leaks - ensures memory freed exactly once
    ~Person() {
        if (name) {
            // Clear sensitive data before freeing
            std::memset(name, 0, 101);
            delete[] name;
            name = nullptr;
        }
        if (address) {
            // Clear sensitive data before freeing
            std::memset(address, 0, 201);
            delete[] address;
            address = nullptr;
        }
    }
    
    // Delete copy constructor and assignment to prevent double-free
    Person(const Person&) = delete;
    Person& operator=(const Person&) = delete;
};

// Memory allocation function that allocates a Person structure
// Returns a pointer to the allocated memory or nullptr on failure
Person* allocatePerson() {
    // Allocate memory using new with exception handling
    Person* person = nullptr;
    
    try {
        person = new Person();
        // Check allocation result
        if (!person) {
            return nullptr;
        }
    } catch (const std::bad_alloc& e) {
        // Handle allocation failure
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return nullptr;
    }
    
    return person;
}

// Overloaded function to allocate and initialize Person
Person* allocatePerson(const char* name, int age, const char* address) {
    // Input validation - check for null pointers
    if (!name || !address) {
        std::cerr << "Invalid input: null pointer provided" << std::endl;
        return nullptr;
    }
    
    // Validate string lengths to prevent buffer overflow
    size_t nameLen = std::strlen(name);
    size_t addrLen = std::strlen(address);
    
    if (nameLen > 100) {
        std::cerr << "Invalid input: name too long" << std::endl;
        return nullptr;
    }
    
    if (addrLen > 200) {
        std::cerr << "Invalid input: address too long" << std::endl;
        return nullptr;
    }
    
    // Validate age range
    if (age < 0 || age > 150) {
        std::cerr << "Invalid input: age out of range" << std::endl;
        return nullptr;
    }
    
    Person* person = allocatePerson();
    if (!person) {
        return nullptr;
    }
    
    // Safe string copy with null termination guarantee
    std::strncpy(person->name, name, 100);
    person->name[100] = '\\0'; // Ensure null termination
    
    person->age = age;
    
    std::strncpy(person->address, address, 200);
    person->address[200] = '\\0'; // Ensure null termination
    
    return person;
}

// Function to safely deallocate Person
void deallocatePerson(Person* person) {
    if (person) {
        delete person;
    }
}

int main() {
    // Test case 1: Basic allocation
    std::cout << "Test 1: Basic allocation" << std::endl;
    Person* p1 = allocatePerson();
    if (p1) {
        std::cout << "Person allocated: Success" << std::endl;
        deallocatePerson(p1);
    } else {
        std::cout << "Person allocation: Failed" << std::endl;
    }
    
    // Test case 2: Allocation with initialization
    std::cout << "\\nTest 2: Allocation with initialization" << std::endl;
    Person* p2 = allocatePerson("John Doe", 30, "123 Main St");
    if (p2) {
        std::cout << "Name: " << p2->name << ", Age: " << p2->age 
                  << ", Address: " << p2->address << std::endl;
        deallocatePerson(p2);
    }
    
    // Test case 3: Multiple allocations
    std::cout << "\\nTest 3: Multiple allocations" << std::endl;
    Person* p3 = allocatePerson("Alice Smith", 25, "456 Oak Ave");
    Person* p4 = allocatePerson("Bob Johnson", 45, "789 Pine Rd");
    if (p3 && p4) {
        std::cout << "p3 - Name: " << p3->name << ", Age: " << p3->age << std::endl;
        std::cout << "p4 - Name: " << p4->name << ", Age: " << p4->age << std::endl;
        deallocatePerson(p3);
        deallocatePerson(p4);
    }
    
    // Test case 4: Invalid input handling - null pointer
    std::cout << "\\nTest 4: Invalid input handling" << std::endl;
    Person* p5 = allocatePerson(nullptr, 30, "Address");
    if (!p5) {
        std::cout << "Correctly rejected null input" << std::endl;
    }
    
    // Test case 5: Boundary value testing - invalid age
    std::cout << "\\nTest 5: Boundary value testing" << std::endl;
    Person* p6 = allocatePerson("Valid Name", -5, "Address");
    if (!p6) {
        std::cout << "Correctly rejected invalid age" << std::endl;
    }
    
    Person* p7 = allocatePerson("Edge Case", 0, "Valid Address");
    if (p7) {
        std::cout << "Allocated person with age 0: Success" << std::endl;
        deallocatePerson(p7);
    }
    
    return 0;
}
