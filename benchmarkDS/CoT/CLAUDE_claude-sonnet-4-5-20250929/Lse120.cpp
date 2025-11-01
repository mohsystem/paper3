
#include <iostream>
#include <cstring>
#include <memory>
#include <limits>

void allocateAndCopy(const char* data) {
    if (data == nullptr) {
        std::cout << "Error: Input data is null" << std::endl;
        return;
    }
    
    // Limit size to prevent excessive memory allocation
    const size_t MAX_SIZE = 1024;
    size_t dataLen = std::strlen(data);
    
    if (dataLen > MAX_SIZE) {
        std::cout << "Error: Input data exceeds maximum size" << std::endl;
        return;
    }
    
    try {
        // Allocate first chunk with bounds checking
        std::unique_ptr<char[]> chunk1(new char[dataLen + 1]);
        
        // Allocate second chunk with same size
        std::unique_ptr<char[]> chunk2(new char[dataLen + 1]);
        
        // Safely copy contents using secure function
        std::strncpy(chunk1.get(), data, dataLen);
        chunk1[dataLen] = '\\0';
        
        std::strncpy(chunk2.get(), chunk1.get(), dataLen);
        chunk2[dataLen] = '\\0';
        
        // Print contents of second chunk
        std::cout << "Contents of second chunk: " << chunk2.get() << std::endl;
        
    } catch (const std::bad_alloc& e) {
        std::cout << "Error: Memory allocation failed" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Normal string
    std::cout << "Test 1:" << std::endl;
    allocateAndCopy("Hello World");
    
    // Test case 2: Empty string
    std::cout << "\\nTest 2:" << std::endl;
    allocateAndCopy("");
    
    // Test case 3: String with special characters
    std::cout << "\\nTest 3:" << std::endl;
    allocateAndCopy("Test@123!#$");
    
    // Test case 4: Null input
    std::cout << "\\nTest 4:" << std::endl;
    allocateAndCopy(nullptr);
    
    // Test case 5: Longer string
    std::cout << "\\nTest 5:" << std::endl;
    allocateAndCopy("Secure memory allocation test");
    
    return 0;
}
