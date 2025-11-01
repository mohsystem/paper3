
#include <iostream>
#include <cstring>
#include <vector>

class Lse1 {
public:
    /**
     * Safely copies bytes from source to destination
     * @param dest Destination buffer
     * @param destSize Size of destination buffer
     * @param src Source buffer
     * @param srcSize Size of source buffer
     * @param numBytes Number of bytes to copy
     * @return true if successful, false otherwise
     */
    static bool safeCopyBytes(void* dest, size_t destSize, const void* src, size_t srcSize, size_t numBytes) {
        // Input validation
        if (dest == nullptr || src == nullptr) {
            std::cerr << "Error: Null pointer provided" << std::endl;
            return false;
        }
        
        if (numBytes == 0) {
            return true; // Nothing to copy
        }
        
        // Bounds checking
        if (numBytes > srcSize) {
            std::cerr << "Error: Source buffer too small" << std::endl;
            return false;
        }
        
        if (numBytes > destSize) {
            std::cerr << "Error: Destination buffer too small" << std::endl;
            return false;
        }
        
        // Check for overflow
        if (numBytes > SIZE_MAX) {
            std::cerr << "Error: Byte count overflow" << std::endl;
            return false;
        }
        
        // Safe copy operation using memmove (handles overlapping memory)
        std::memmove(dest, src, numBytes);
        return true;
    }
    
    static void printBuffer(const unsigned char* buffer, size_t size) {
        std::cout << "[";
        for (size_t i = 0; i < size; i++) {
            std::cout << static_cast<int>(buffer[i]);
            if (i < size - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
};

int main() {
    std::cout << "=== Test Case 1: Normal copy ===" << std::endl;
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dest1[5] = {0};
    bool result1 = Lse1::safeCopyBytes(dest1, sizeof(dest1), src1, sizeof(src1), 5);
    std::cout << "Result: " << (result1 ? "true" : "false") << ", Dest: ";
    Lse1::printBuffer(dest1, sizeof(dest1));
    
    std::cout << "\\n=== Test Case 2: Partial copy ===" << std::endl;
    unsigned char src2[] = {10, 20, 30, 40, 50};
    unsigned char dest2[10] = {0};
    bool result2 = Lse1::safeCopyBytes(dest2, sizeof(dest2), src2, sizeof(src2), 3);
    std::cout << "Result: " << (result2 ? "true" : "false") << ", Dest: ";
    Lse1::printBuffer(dest2, sizeof(dest2));
    
    std::cout << "\\n=== Test Case 3: Null source ===" << std::endl;
    unsigned char dest3[5] = {0};
    bool result3 = Lse1::safeCopyBytes(dest3, sizeof(dest3), nullptr, 0, 5);
    std::cout << "Result: " << (result3 ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Buffer overflow attempt ===" << std::endl;
    unsigned char src4[] = {1, 2, 3};
    unsigned char dest4[2] = {0};
    bool result4 = Lse1::safeCopyBytes(dest4, sizeof(dest4), src4, sizeof(src4), 3);
    std::cout << "Result: " << (result4 ? "true" : "false") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Zero bytes ===" << std::endl;
    unsigned char src5[] = {1, 2, 3};
    unsigned char dest5[3] = {0};
    bool result5 = Lse1::safeCopyBytes(dest5, sizeof(dest5), src5, sizeof(src5), 0);
    std::cout << "Result: " << (result5 ? "true" : "false") << ", Dest: ";
    Lse1::printBuffer(dest5, sizeof(dest5));
    
    return 0;
}
