
#include <iostream>
#include <cstring>
#include <stdexcept>

void copyBytes(char* destination, const char* source, size_t numBytes) {
    if (destination == nullptr || source == nullptr) {
        throw std::invalid_argument("Destination and source pointers cannot be null");
    }
    
    if (numBytes > strlen(source)) {
        throw std::invalid_argument("Number of bytes exceeds source length");
    }
    
    memcpy(destination, source, numBytes);
}

int main() {
    // Test case 1: Copy "Hello" to buffer
    char buffer1[10] = {0};
    const char* source1 = "Hello";
    copyBytes(buffer1, source1, strlen(source1));
    std::cout << "Test 1: " << buffer1 << std::endl;
    
    // Test case 2: Copy partial string
    char buffer2[10] = {0};
    const char* source2 = "Hello";
    copyBytes(buffer2, source2, 3);
    std::cout << "Test 2: " << buffer2 << std::endl;
    
    // Test case 3: Copy to exact size buffer
    char buffer3[6] = {0};
    const char* source3 = "Hello";
    copyBytes(buffer3, source3, 5);
    std::cout << "Test 3: " << buffer3 << std::endl;
    
    // Test case 4: Copy empty string
    char buffer4[10] = {0};
    const char* source4 = "";
    copyBytes(buffer4, source4, 0);
    std::cout << "Test 4: (empty)" << std::endl;
    
    // Test case 5: Copy another string
    char buffer5[15] = {0};
    const char* source5 = "World";
    copyBytes(buffer5, source5, strlen(source5));
    std::cout << "Test 5: " << buffer5 << std::endl;
    
    return 0;
}
