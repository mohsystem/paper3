
#include <iostream>
#include <cstring>
#include <algorithm>

void copyBytes(char* destination, const char* source, int numBytes) {
    for (int i = 0; i < numBytes; i++) {
        destination[i] = source[i];
    }
}

int main() {
    // Test case 1: Normal copy
    char dest1[20] = {0};
    const char* src1 = "Hello World";
    copyBytes(dest1, src1, strlen(src1));
    std::cout << "Test 1: " << dest1 << std::endl;
    
    // Test case 2: Copy fewer bytes than source
    char dest2[20] = {0};
    const char* src2 = "Hello World";
    copyBytes(dest2, src2, 5);
    std::cout << "Test 2: " << dest2 << std::endl;
    
    // Test case 3: Empty source
    char dest3[20] = {0};
    const char* src3 = "";
    copyBytes(dest3, src3, 0);
    std::cout << "Test 3: " << dest3 << std::endl;
    
    // Test case 4: Copy with exact buffer size
    char dest4[6] = {0};
    const char* src4 = "12345";
    copyBytes(dest4, src4, 5);
    std::cout << "Test 4: " << dest4 << std::endl;
    
    // Test case 5: Larger numBytes than source (unsafe but demonstrating)
    char dest5[20] = {0};
    const char* src5 = "Test";
    copyBytes(dest5, src5, strlen(src5));
    std::cout << "Test 5: " << dest5 << std::endl;
    
    return 0;
}
