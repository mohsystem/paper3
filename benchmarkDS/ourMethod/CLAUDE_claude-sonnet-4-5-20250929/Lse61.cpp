
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

// Demonstrates proper memory management pattern
// Allocates and deallocates buffer correctly - no double-free
void allocateAndFreeBuffer(size_t bufferSize, int iterations) {
    // Validate input to prevent negative or excessive allocations
    if (bufferSize == 0 || bufferSize > 100 * 1024 * 1024) { // Max 100MB
        throw std::invalid_argument("Invalid buffer size");
    }
    if (iterations <= 0 || iterations > 1000) {
        throw std::invalid_argument("Invalid iteration count");
    }
    
    // Correct pattern: allocate and free within each iteration
    // This prevents double-free vulnerability (CWE-415)
    for (int i = 0; i < iterations; i++) {
        // Allocate buffer for this iteration
        unsigned char* buffer = static_cast<unsigned char*>(std::malloc(bufferSize));
        
        // Check allocation success
        if (buffer == nullptr) {
            throw std::bad_alloc();
        }
        
        // Use buffer (example: initialize to demonstrate allocation worked)
        if (bufferSize > 0) {
            buffer[0] = static_cast<unsigned char>(i);
        }
        
        // Free the buffer exactly once per allocation
        // This is the correct pattern - one free per malloc
        std::free(buffer);
        
        // Set pointer to nullptr after free to prevent use-after-free
        buffer = nullptr;
    }
}

int main() {
    try {
        // Test case 1: Small buffer, few iterations
        allocateAndFreeBuffer(1024, 5);
        std::cout << "Test 1 passed: 1KB buffer, 5 iterations" << std::endl;
        
        // Test case 2: Medium buffer, moderate iterations
        allocateAndFreeBuffer(8192, 10);
        std::cout << "Test 2 passed: 8KB buffer, 10 iterations" << std::endl;
        
        // Test case 3: Larger buffer, single iteration
        allocateAndFreeBuffer(65536, 1);
        std::cout << "Test 3 passed: 64KB buffer, 1 iteration" << std::endl;
        
        // Test case 4: Small buffer, many iterations
        allocateAndFreeBuffer(512, 100);
        std::cout << "Test 4 passed: 512B buffer, 100 iterations" << std::endl;
        
        // Test case 5: Edge case - minimum valid size
        allocateAndFreeBuffer(1, 3);
        std::cout << "Test 5 passed: 1B buffer, 3 iterations" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
