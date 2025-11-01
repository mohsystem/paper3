
#include <iostream>
#include <cstring>
#include <stdexcept>

class Lse2 {
public:
    static void copyBytes(unsigned char* destination, const unsigned char* source, size_t numBytes) {
        if (destination == nullptr || source == nullptr) {
            throw std::invalid_argument("Destination and source cannot be null");
        }
        if (numBytes < 0) {
            throw std::invalid_argument("Number of bytes cannot be negative");
        }
        
        for (size_t i = 0; i < numBytes; i++) {
            destination[i] = source[i];
        }
    }
};

int main() {
    // Test case 1: Normal copy
    unsigned char dest1[20] = {0};
    const unsigned char* src1 = (const unsigned char*)"Hello World";
    Lse2::copyBytes(dest1, src1, 11);
    std::cout << "Test 1: " << dest1 << std::endl;
    
    // Test case 2: Partial copy
    unsigned char dest2[10] = {0};
    const unsigned char* src2 = (const unsigned char*)"Testing";
    Lse2::copyBytes(dest2, src2, 4);
    std::cout << "Test 2: " << dest2 << std::endl;
    
    // Test case 3: Copy with exact size
    unsigned char dest3[6] = {0};
    const unsigned char* src3 = (const unsigned char*)"Exact";
    Lse2::copyBytes(dest3, src3, 5);
    std::cout << "Test 3: " << dest3 << std::endl;
    
    // Test case 4: Copy zero bytes
    unsigned char dest4[10] = {0};
    const unsigned char* src4 = (const unsigned char*)"Zero";
    Lse2::copyBytes(dest4, src4, 0);
    std::cout << "Test 4: Empty copy successful" << std::endl;
    
    // Test case 5: Copy single byte
    unsigned char dest5[10] = {0};
    const unsigned char* src5 = (const unsigned char*)"A";
    Lse2::copyBytes(dest5, src5, 1);
    std::cout << "Test 5: " << dest5 << std::endl;
    
    return 0;
}
