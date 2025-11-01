
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <algorithm>

// Portable explicit zero function to ensure memory is cleared
// Prevents compiler optimization from removing the zeroing operation
static void explicit_zero(void* ptr, size_t size) {
    if (ptr == nullptr || size == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (size--) {
        *p++ = 0;
    }
}

void demonstrateMemoryCopy(size_t size) {
    // Input validation: check for reasonable size and overflow
    if (size == 0 || size > 1024 * 1024) { // Limit to 1MB
        std::cerr << "Invalid size" << std::endl;
        return;
    }
    
    // Allocate first chunk with nullptr check
    unsigned char* chunk1 = static_cast<unsigned char*>(std::malloc(size));
    if (chunk1 == nullptr) {
        std::cerr << "Memory allocation failed for chunk1" << std::endl;
        return;
    }
    
    // Initialize chunk1 with sample data
    for (size_t i = 0; i < size; i++) {
        chunk1[i] = static_cast<unsigned char>(i % 256);
    }
    
    // Allocate second chunk with nullptr check
    unsigned char* chunk2 = static_cast<unsigned char*>(std::malloc(size));
    if (chunk2 == nullptr) {
        explicit_zero(chunk1, size); // Clear before free
        std::free(chunk1);
        std::cerr << "Memory allocation failed for chunk2" << std::endl;
        return;
    }
    
    // Securely copy with bounds checking (memcpy is safe when sizes match)
    std::memcpy(chunk2, chunk1, size);
    
    // Print contents of chunk2 (limited output)
    std::cout << "Contents of chunk2 after copy: ";
    for (size_t i = 0; i < std::min(size, static_cast<size_t>(10)); i++) {
        std::cout << static_cast<int>(chunk2[i]) << " ";
    }
    std::cout << std::endl;
    
    // Clear chunk1 before freeing (important for sensitive data)
    explicit_zero(chunk1, size);
    std::free(chunk1);
    chunk1 = nullptr; // Prevent use after free
    
    // Print contents of chunk2 again (still valid)
    std::cout << "Contents of chunk2 after chunk1 freed: ";
    for (size_t i = 0; i < std::min(size, static_cast<size_t>(10)); i++) {
        std::cout << static_cast<int>(chunk2[i]) << " ";
    }
    std::cout << std::endl;
    
    // Clear and free chunk2
    explicit_zero(chunk2, size);
    std::free(chunk2);
    chunk2 = nullptr; // Prevent use after free
}

int main() {
    // Test case 1: Small allocation
    std::cout << "Test 1: Size 10" << std::endl;
    demonstrateMemoryCopy(10);
    
    // Test case 2: Medium allocation
    std::cout << "\\nTest 2: Size 100" << std::endl;
    demonstrateMemoryCopy(100);
    
    // Test case 3: Larger allocation
    std::cout << "\\nTest 3: Size 1000" << std::endl;
    demonstrateMemoryCopy(1000);
    
    // Test case 4: Edge case - size 1
    std::cout << "\\nTest 4: Size 1" << std::endl;
    demonstrateMemoryCopy(1);
    
    // Test case 5: Invalid case - size 0
    std::cout << "\\nTest 5: Size 0 (invalid)" << std::endl;
    demonstrateMemoryCopy(0);
    
    return 0;
}
