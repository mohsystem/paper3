
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <memory>

#define MAX_BUFFER_SIZE 1024

class Lse59 {
public:
    static void allocateAndFillBuffer(int bufferSize) {
        // Validate input
        if (bufferSize <= 0) {
            throw std::invalid_argument("Buffer size must be positive");
        }
        if (bufferSize > MAX_BUFFER_SIZE) {
            throw std::invalid_argument("Buffer size exceeds maximum allowed size");
        }
        
        // Allocate buffer on stack
        char buffer[MAX_BUFFER_SIZE];
        
        // Fill buffer with 'A' using safe memory operation
        memset(buffer, 'A', bufferSize);
        buffer[bufferSize] = '\\0'; // Null terminate
        
        std::cout << "Buffer content: " << std::string(buffer, bufferSize) << std::endl;
    }
};

int main() {
    // Test case 1: Normal size
    std::cout << "Test 1 (size 10): ";
    Lse59::allocateAndFillBuffer(10);
    
    // Test case 2: Small size
    std::cout << "Test 2 (size 1): ";
    Lse59::allocateAndFillBuffer(1);
    
    // Test case 3: Medium size
    std::cout << "Test 3 (size 50): ";
    Lse59::allocateAndFillBuffer(50);
    
    // Test case 4: Larger size
    std::cout << "Test 4 (size 500): ";
    Lse59::allocateAndFillBuffer(500);
    
    // Test case 5: Edge case - invalid size
    std::cout << "Test 5: ";
    try {
        Lse59::allocateAndFillBuffer(0);
    } catch (const std::invalid_argument& e) {
        std::cout << "Caught expected exception - " << e.what() << std::endl;
    }
    
    return 0;
}
