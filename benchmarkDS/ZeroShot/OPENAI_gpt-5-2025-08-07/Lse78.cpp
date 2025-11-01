#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>

static const std::size_t MAX_SAFE_SIZE = 16u * 1024u * 1024u; // 16 MB cap

// Secure memory allocation: returns pointer to zeroed block or nullptr on error
void* allocate_memory(std::size_t size) {
    if (size == 0 || size > MAX_SAFE_SIZE) {
        return nullptr;
    }
    void* p = std::calloc(1, size); // zero-initialized
    return p; // nullptr if allocation failed
}

struct Person {
    int status;
    Person() : status(0) {}
};

// Allocates a new Person and sets status to 0
Person* allocate_person() {
    Person* p = new (std::nothrow) Person();
    return p;
}

int main() {
    // 5 test cases

    // Test 1: Allocate 1024 bytes
    void* t1 = allocate_memory(1024);
    std::cout << "Test1 allocate_memory(1024): " << (t1 != nullptr ? "ok" : "null") << "\n";
    std::free(t1);

    // Test 2: Allocate Person and verify status
    Person* p1 = allocate_person();
    std::cout << "Test2 allocate_person(): status=" << (p1 ? p1->status : -1) << "\n";
    delete p1;

    // Test 3: Allocate 0 bytes (invalid per guard)
    void* t3 = allocate_memory(0);
    std::cout << "Test3 allocate_memory(0): " << (t3 != nullptr ? "ok" : "null") << "\n";
    std::free(t3);

    // Test 4: Allocate excessively large size
    void* t4 = allocate_memory(MAX_SAFE_SIZE + 1);
    std::cout << "Test4 allocate_memory(>MAX): " << (t4 != nullptr ? "ok" : "null") << "\n";
    std::free(t4);

    // Test 5: Another valid allocation
    void* t5 = allocate_memory(2048);
    std::cout << "Test5 allocate_memory(2048): " << (t5 != nullptr ? "ok" : "null") << "\n";
    std::free(t5);

    return 0;
}