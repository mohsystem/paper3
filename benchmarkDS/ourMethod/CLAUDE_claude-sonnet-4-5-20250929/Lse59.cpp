
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <memory>

// Allocates a buffer and fills it with 'A'
// Parameters:
//   buffer_size: the size of the buffer to allocate
// Returns: pointer to the filled buffer (caller must manage)
char* fillBufferWithA(int buffer_size) {
    // Validate input: buffer size must be positive and within reasonable limits
    // to prevent excessive memory usage (max 10MB for stack safety)
    if (buffer_size <= 0 || buffer_size > 10 * 1024 * 1024) {
        throw std::invalid_argument("Buffer size must be positive and not exceed 10MB");
    }
    
    // Check for integer overflow before allocation
    if (buffer_size > SIZE_MAX / sizeof(char)) {
        throw std::overflow_error("Buffer size would cause integer overflow");
    }
    
    // Allocate buffer on the stack using Variable Length Array (VLA) alternative
    // Since VLA is not standard C++, we use a local array with maximum size
    // For true stack allocation with variable size, we demonstrate with alloca-style
    // but return a copy to heap for safety
    char* buffer = new(std::nothrow) char[buffer_size];
    
    // Check allocation result
    if (buffer == nullptr) {
        throw std::bad_alloc();
    }
    
    // Initialize buffer pointer for safety
    // Fill buffer with 'A' - using memset for bounds-safe operation
    // memset is safe here because we control the size parameter
    std::memset(buffer, 'A', buffer_size);
    
    return buffer;
}

int main() {
    // Test case 1: Small buffer
    try {
        char* result1 = fillBufferWithA(10);
        std::cout << "Test 1 - Size 10: ";
        std::cout.write(result1, 10);
        std::cout << std::endl;
        delete[] result1; // Free exactly once
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Medium buffer
    try {
        char* result2 = fillBufferWithA(100);
        std::cout << "Test 2 - Size 100: First 10 chars: ";
        std::cout.write(result2, 10);
        std::cout << std::endl;
        delete[] result2; // Free exactly once
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Large buffer
    try {
        char* result3 = fillBufferWithA(1000);
        std::cout << "Test 3 - Size 1000: Buffer allocated successfully" << std::endl;
        delete[] result3; // Free exactly once
    } catch (const std::exception& e) {
        std::cout << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Edge case - size 1
    try {
        char* result4 = fillBufferWithA(1);
        std::cout << "Test 4 - Size 1: ";
        std::cout.write(result4, 1);
        std::cout << std::endl;
        delete[] result4; // Free exactly once
    } catch (const std::exception& e) {
        std::cout << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid input - negative size
    try {
        char* result5 = fillBufferWithA(-1);
        std::cout << "Test 5 - Negative size should fail" << std::endl;
        delete[] result5;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Correctly rejected negative size: " << e.what() << std::endl;
    }
    
    return 0;
}
