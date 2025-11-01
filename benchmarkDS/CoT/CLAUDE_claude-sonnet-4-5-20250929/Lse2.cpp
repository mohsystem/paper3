
#include <iostream>
#include <cstring>
#include <cstdio>

bool secureCopy(void* destination, const void* source, size_t destSize, size_t numBytes) {
    // Input validation
    if (destination == nullptr || source == nullptr) {
        std::cerr << "Error: Null pointer provided" << std::endl;
        return false;
    }
    
    if (numBytes > destSize) {
        std::cerr << "Error: Copy size exceeds destination capacity" << std::endl;
        return false;
    }
    
    // Safe copy operation using memcpy
    std::memcpy(destination, source, numBytes);
    return true;
}

int main() {
    // Test case 1: Normal copy
    char dest1[20] = {0};
    const char* src1 = "Hello World";
    std::cout << "Test 1: " << secureCopy(dest1, src1, sizeof(dest1), std::strlen(src1) + 1) << std::endl;
    std::cout << "Result: " << dest1 << std::endl;
    
    // Test case 2: Partial copy
    char dest2[10] = {0};
    const char* src2 = "Test";
    std::cout << "\\nTest 2: " << secureCopy(dest2, src2, sizeof(dest2), 4) << std::endl;
    std::cout << "Result: " << dest2 << std::endl;
    
    // Test case 3: Buffer overflow prevention
    char dest3[5] = {0};
    const char* src3 = "Too long message";
    std::cout << "\\nTest 3: " << secureCopy(dest3, src3, sizeof(dest3), std::strlen(src3) + 1) << std::endl;
    
    // Test case 4: Null source
    char dest4[10] = {0};
    std::cout << "\\nTest 4: " << secureCopy(dest4, nullptr, sizeof(dest4), 5) << std::endl;
    
    // Test case 5: Zero bytes
    char dest5[10] = {0};
    const char* src5 = "Data";
    std::cout << "\\nTest 5: " << secureCopy(dest5, src5, sizeof(dest5), 0) << std::endl;
    
    return 0;
}
