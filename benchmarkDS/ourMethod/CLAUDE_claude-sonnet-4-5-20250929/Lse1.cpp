
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <limits>

/**
 * Copies a specified number of bytes from source to destination.
 * 
 * @param destination Pointer to destination buffer
 * @param source Pointer to source buffer
 * @param numBytes Number of bytes to copy
 * @return true if copy successful, false otherwise
 */
bool copyBytes(void* destination, const void* source, size_t numBytes) {
    // Input validation: check for null pointers
    if (destination == nullptr || source == nullptr) {
        return false;
    }
    
    // Input validation: check for zero bytes (valid but no-op)
    if (numBytes == 0) {
        return true;
    }
    
    // Prevent potential integer overflow in pointer arithmetic
    // Check if numBytes would overflow when added to pointers
    if (numBytes > SIZE_MAX) {
        return false;
    }
    
    // Additional safety: check for pointer overlap which could cause issues
    // If buffers overlap, behavior is undefined in memcpy
    const unsigned char* src_ptr = static_cast<const unsigned char*>(source);
    unsigned char* dest_ptr = static_cast<unsigned char*>(destination);
    
    // Check for overlapping regions
    if ((dest_ptr >= src_ptr && dest_ptr < src_ptr + numBytes) ||
        (src_ptr >= dest_ptr && src_ptr < dest_ptr + numBytes)) {
        // Use memmove for overlapping regions instead
        std::memmove(destination, source, numBytes);
        return true;
    }
    
    // Safe copy operation - memcpy does not perform bounds checking
    // Caller must ensure buffers are large enough
    std::memcpy(destination, source, numBytes);
    return true;
}

int main() {
    // Test case 1: Normal copy
    unsigned char dest1[10] = {0};
    unsigned char src1[] = {1, 2, 3, 4, 5};
    bool result1 = copyBytes(dest1, src1, 5);
    std::cout << "Test 1 - Normal copy: " << result1 << ", Dest: ";
    for (int i = 0; i < 10; i++) std::cout << static_cast<int>(dest1[i]) << " ";
    std::cout << std::endl;
    
    // Test case 2: Copy with numBytes = 0
    unsigned char dest2[5] = {0};
    unsigned char src2[] = {10, 20, 30};
    bool result2 = copyBytes(dest2, src2, 0);
    std::cout << "Test 2 - Zero bytes: " << result2 << std::endl;
    
    // Test case 3: Overlapping regions (handled by memmove)
    unsigned char buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    bool result3 = copyBytes(buffer + 2, buffer, 5);
    std::cout << "Test 3 - Overlapping copy: " << result3 << ", Buffer: ";
    for (int i = 0; i < 10; i++) std::cout << static_cast<int>(buffer[i]) << " ";
    std::cout << std::endl;
    
    // Test case 4: Null source (should fail)
    unsigned char dest4[5] = {0};
    bool result4 = copyBytes(dest4, nullptr, 3);
    std::cout << "Test 4 - Null source: " << result4 << std::endl;
    
    // Test case 5: Null destination (should fail)
    unsigned char src5[] = {1, 2, 3};
    bool result5 = copyBytes(nullptr, src5, 3);
    std::cout << "Test 5 - Null destination: " << result5 << std::endl;
    
    return 0;
}
