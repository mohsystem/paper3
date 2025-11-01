
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <cstdlib>

// Maximum name length to prevent excessive memory allocation
constexpr size_t MAX_NAME_LENGTH = 255;

class Person {
private:
    int status;
    char* name;
    size_t name_capacity;

public:
    // Constructor with validation
    Person(int status_val, const char* name_val) : status(0), name(nullptr), name_capacity(0) {
        // Input validation: check status range
        if (status_val < 0 || status_val > 1000) {
            throw std::invalid_argument("Status must be between 0 and 1000");
        }
        
        // Input validation: check name is not null
        if (name_val == nullptr) {
            throw std::invalid_argument("Name cannot be null");
        }
        
        // Check name length to prevent excessive allocation
        size_t name_len = strnlen(name_val, MAX_NAME_LENGTH + 1);
        if (name_len > MAX_NAME_LENGTH) {
            throw std::invalid_argument("Name exceeds maximum length");
        }
        
        // Safe memory allocation with overflow check
        name_capacity = name_len + 1;
        name = static_cast<char*>(calloc(name_capacity, sizeof(char)));
        
        // Check allocation result
        if (name == nullptr) {
            throw std::bad_alloc();
        }
        
        // Safe string copy with bounds checking
        strncpy(name, name_val, name_capacity - 1);
        name[name_capacity - 1] = '\\0';  // Ensure null termination
        
        this->status = status_val;
    }
    
    // Destructor to free allocated memory
    ~Person() {
        clear();
    }
    
    // Delete copy constructor and assignment to prevent double-free
    Person(const Person&) = delete;
    Person& operator=(const Person&) = delete;
    
    // Getters
    int getStatus() const { return status; }
    const char* getName() const { return name; }
    
    // Clear sensitive data securely
    void clear() {
        if (name != nullptr) {
            // Explicitly zero memory before freeing to clear sensitive data
            explicit_bzero(name, name_capacity);
            free(name);
            name = nullptr;
        }
        name_capacity = 0;
        status = 0;
    }
};

// Factory function to create Person with validated inputs
std::unique_ptr<Person> createPerson(int status, const char* name) {
    // Validation happens in Person constructor
    return std::make_unique<Person>(status, name);
}

int main() {
    // Test case 1: Create person with status 0 and name "John"
    try {
        auto person1 = createPerson(0, "John");
        std::cout << "Test 1 - Status: " << person1->getStatus() 
                  << ", Name: " << person1->getName() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Create person with different status
    try {
        auto person2 = createPerson(1, "Alice");
        std::cout << "Test 2 - Status: " << person2->getStatus() 
                  << ", Name: " << person2->getName() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Create person with empty name
    try {
        auto person3 = createPerson(0, "");
        std::cout << "Test 3 - Status: " << person3->getStatus() 
                  << ", Name: '" << person3->getName() << "'" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Attempt to create with null name (should fail)
    try {
        auto person4 = createPerson(0, nullptr);
        std::cout << "Test 4 - Status: " << person4->getStatus() 
                  << ", Name: " << person4->getName() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 correctly rejected null name: " << e.what() << std::endl;
    }
    
    // Test case 5: Attempt to create with invalid status (should fail)
    try {
        auto person5 = createPerson(-1, "Bob");
        std::cout << "Test 5 - Status: " << person5->getStatus() 
                  << ", Name: " << person5->getName() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 correctly rejected invalid status: " << e.what() << std::endl;
    }
    
    return 0;
}
