
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <new>

// Person structure with status field
struct Person {
    int status;
};

/**
 * Allocates memory for a Person structure and initializes status to 0.
 * 
 * Returns a pointer to the allocated Person, or nullptr on failure.
 * Caller is responsible for freeing the returned pointer.
 */
Person* allocatePerson() {
    // Allocate memory using new (C++ RAII-style allocation)
    // new throws std::bad_alloc on failure, which we catch
    Person* person = nullptr;
    
    try {
        // Allocate memory for Person structure
        person = new Person();
        
        // Initialize status field to 0
        // Using memset for explicit zero initialization
        person->status = 0;
        
    } catch (const std::bad_alloc& e) {
        // Allocation failed, return nullptr (fail closed)
        return nullptr;
    }
    
    return person;
}

/**
 * Safely deallocates a Person structure.
 * Sets the pointer to nullptr after freeing.
 */
void deallocatePerson(Person*& person) {
    if (person != nullptr) {
        // No sensitive data to clear in this simple structure
        delete person;
        person = nullptr;  // Prevent double-free
    }
}

int main() {
    std::cout << "Testing Person allocation:" << std::endl;
    
    // Test case 1: Allocate a single person
    Person* person1 = allocatePerson();
    if (person1 != nullptr) {
        std::cout << "Test 1 - Person allocated with status: " << person1->status << std::endl;
        deallocatePerson(person1);
    } else {
        std::cout << "Test 1 - Allocation failed" << std::endl;
    }
    
    // Test case 2: Allocate another person
    Person* person2 = allocatePerson();
    if (person2 != nullptr) {
        std::cout << "Test 2 - Person allocated with status: " << person2->status << std::endl;
        deallocatePerson(person2);
    } else {
        std::cout << "Test 2 - Allocation failed" << std::endl;
    }
    
    // Test case 3: Verify status is 0
    Person* person3 = allocatePerson();
    if (person3 != nullptr) {
        std::cout << "Test 3 - Status is 0: " << (person3->status == 0 ? "true" : "false") << std::endl;
        deallocatePerson(person3);
    } else {
        std::cout << "Test 3 - Allocation failed" << std::endl;
    }
    
    // Test case 4: Allocate multiple persons
    const int count = 5;
    Person* people[count] = {nullptr};
    int allocated = 0;
    for (int i = 0; i < count; i++) {
        people[i] = allocatePerson();
        if (people[i] != nullptr) {
            allocated++;
        }
    }
    std::cout << "Test 4 - Allocated " << allocated << " persons" << std::endl;
    for (int i = 0; i < count; i++) {
        deallocatePerson(people[i]);
    }
    
    // Test case 5: Verify all have status 0
    Person* person4 = allocatePerson();
    Person* person5 = allocatePerson();
    if (person4 != nullptr && person5 != nullptr) {
        bool allZero = (person4->status == 0) && (person5->status == 0);
        std::cout << "Test 5 - All status fields are 0: " << (allZero ? "true" : "false") << std::endl;
    } else {
        std::cout << "Test 5 - Allocation failed" << std::endl;
    }
    deallocatePerson(person4);
    deallocatePerson(person5);
    
    return 0;
}
