
#include <iostream>
#include <cstring>
#include <string>

class Lse3 {
public:
    /**
     * Securely copies bytes from source to destination with bounds checking
     * @param dest destination buffer
     * @param destSize size of destination buffer
     * @param src source buffer
     * @param srcSize size of source buffer
     * @param numBytes number of bytes to copy
     * @return true if successful, false otherwise
     */
    static bool secureCopy(char* dest, size_t destSize, const char* src, size_t srcSize, size_t numBytes) {
        // Null pointer checks
        if (dest == nullptr || src == nullptr) {
            std::cout << "Error: Null buffer provided" << std::endl;
            return false;
        }
        
        // Check if source has enough bytes
        if (numBytes > srcSize) {
            std::cout << "Error: Source buffer too small" << std::endl;
            return false;
        }
        
        // Check if destination has enough space
        if (numBytes > destSize) {
            std::cout << "Error: Destination buffer too small" << std::endl;
            return false;
        }
        
        // Perform the copy using safe function
        std::memcpy(dest, src, numBytes);
        return true;
    }
};

int main() {
    std::cout << "=== Test Case 1: Copy 'Hello' ===" << std::endl;
    char dest1[10] = {0};
    const char* src1 = "Hello";
    if (Lse3::secureCopy(dest1, sizeof(dest1), src1, std::strlen(src1), std::strlen(src1))) {
        std::cout << "Result: " << dest1 << std::endl;
    }
    
    std::cout << "\\n=== Test Case 2: Buffer too small ===" << std::endl;
    char dest2[3] = {0};
    const char* src2 = "Hello";
    Lse3::secureCopy(dest2, sizeof(dest2), src2, std::strlen(src2), std::strlen(src2));
    
    std::cout << "\\n=== Test Case 3: Zero bytes ===" << std::endl;
    char dest3[10] = {0};
    const char* src3 = "Test";
    if (Lse3::secureCopy(dest3, sizeof(dest3), src3, std::strlen(src3), 0)) {
        std::cout << "Result: Successfully copied 0 bytes" << std::endl;
    }
    
    std::cout << "\\n=== Test Case 4: Null source ===" << std::endl;
    char dest4[10] = {0};
    Lse3::secureCopy(dest4, sizeof(dest4), nullptr, 0, 5);
    
    std::cout << "\\n=== Test Case 5: Exact size match ===" << std::endl;
    char dest5[6] = {0};
    const char* src5 = "World";
    if (Lse3::secureCopy(dest5, sizeof(dest5) - 1, src5, std::strlen(src5), std::strlen(src5))) {
        std::cout << "Result: " << dest5 << std::endl;
    }
    
    return 0;
}
