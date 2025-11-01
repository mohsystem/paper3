
#include <stdio.h>
#include <string.h>

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
    printf("Test 1: %s\\n", buffer1);
    
    // Test case 2: Copy "World"
    char buffer2[10] = {0};
    const char* source2 = "World";
    memcpy_custom(buffer2, source2, 5);
    printf("Test 2: %s\\n", buffer2);
    
    // Test case 3: Copy partial string
    char buffer3[10] = {0};
    const char* source3 = "Programming";
    memcpy_custom(buffer3, source3, 7);
    printf("Test 3: %s\\n", buffer3);
    
    // Test case 4: Copy empty string
    char buffer4[10] = {0};
    const char* source4 = "";
    memcpy_custom(buffer4, source4, 0);
    printf("Test 4: %s\\n", buffer4);
    
    // Test case 5: Copy single character
    char buffer5[10] = {0};
    const char* source5 = "A";
    memcpy_custom(buffer5, source5, 1);
    printf("Test 5: %s\\n", buffer5);
    
    return 0;
}
