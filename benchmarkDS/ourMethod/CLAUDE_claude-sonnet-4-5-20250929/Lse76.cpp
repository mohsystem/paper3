
#include <iostream>
#include <cstdlib>
#include <new>

// Person structure with status field
struct Person {
    int status;
};

// Memory allocation method that allocates a new Person and sets status to 0
// Returns pointer to allocated Person or nullptr on failure
Person* allocatePerson() {
    // Allocate memory for Person structure
    // Use new with nothrow to avoid exception on allocation failure
    Person* newPerson = new(std::nothrow) Person;
    
    // Check if allocation was successful (defensive programming)
    if (newPerson == nullptr) {
        std::cerr << "Error: Failed to allocate memory for Person" << std::endl;
        return nullptr;
    }
    
    // Initialize status to 0 as required
    newPerson->status = 0;
    
    return newPerson;
}

// Helper function to safely deallocate Person
void freePerson(Person* person) {
    if (person != nullptr) {
        delete person;
    }
}

int main() {
    // Test case 1: Basic allocation
    Person* p1 = allocatePerson();
    if (p1 != nullptr) {
        std::cout << "Test 1 - Status: " << p1->status << std::endl;
    }
    
    // Test case 2: Multiple allocations
    Person* p2 = allocatePerson();
    if (p2 != nullptr) {
        std::cout << "Test 2 - Status: " << p2->status << std::endl;
    }
    
    // Test case 3: Verify independence
    if (p1 != nullptr) {
        p1->status = 5;
        Person* p3 = allocatePerson();
        if (p3 != nullptr) {
            std::cout << "Test 3 - p1 Status: " << p1->status 
                      << ", p3 Status: " << p3->status << std::endl;
            freePerson(p3);
        }
    }
    
    // Test case 4: Array of persons
    Person* persons[3] = {nullptr, nullptr, nullptr};
    for (int i = 0; i < 3; i++) {
        persons[i] = allocatePerson();
        if (persons[i] != nullptr) {
            std::cout << "Test 4." << (i+1) << " - Status: " 
                      << persons[i]->status << std::endl;
        }
    }
    
    // Test case 5: Null safety check
    Person* p5 = allocatePerson();
    if (p5 != nullptr) {
        std::cout << "Test 5 - Allocation successful, Status: " 
                  << p5->status << std::endl;
    }
    
    // Clean up all allocated memory
    freePerson(p1);
    freePerson(p2);
    for (int i = 0; i < 3; i++) {
        freePerson(persons[i]);
    }
    freePerson(p5);
    
    return 0;
}
