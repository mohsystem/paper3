
#include <iostream>
#include <cstring>

void memcpy_custom(void* dest, const void* src, size_t numBytes) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    for (size_t i = 0; i < numBytes; i++) {
        d[i] = s[i];
    }
}

int main() {
    // Test case 1: Copy "Hello"
    char buffer1[10] = {0};
    const char* source1 = "Hello";
    memcpy_custom(buffer1, source1, 5);
    std::cout << "Test 1: " << buffer1 << std::endl;
    
    // Test case 2: Copy "World"
    char buffer2[10] = {0};
    const char* source2 = "World";
    memcpy_custom(buffer2, source2, 5);
    std::cout << "Test 2: " << buffer2 << std::endl;
    
    // Test case 3: Copy partial string
    char buffer3[10] = {0};
    const char* source3 = "Programming";
    memcpy_custom(buffer3, source3, 7);
    std::cout << "Test 3: " << buffer3 << std::endl;
    
    // Test case 4: Copy empty string
    char buffer4[10] = {0};
    const char* source4 = "";
    memcpy_custom(buffer4, source4, 0);
    std::cout << "Test 4: " << buffer4 << std::endl;
    
    // Test case 5: Copy single character
    char buffer5[10] = {0};
    const char* source5 = "A";
    memcpy_custom(buffer5, source5, 1);
    std::cout << "Test 5: " << buffer5 << std::endl;
    
    return 0;
}
