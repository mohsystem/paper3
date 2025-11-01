
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void copyBytes(char* destination, const char* source, size_t numBytes) {
    if (destination == NULL || source == NULL) {
        fprintf(stderr, "Error: Destination and source pointers cannot be null\\n");
        return;
    }
    
    if (numBytes > strlen(source)) {
        fprintf(stderr, "Error: Number of bytes exceeds source length\\n");
        return;
    }
    
    memcpy(destination, source, numBytes);
}

int main() {
    // Test case 1: Copy "Hello" to buffer
    char buffer1[10] = {0};
    const char* source1 = "Hello";
    copyBytes(buffer1, source1, strlen(source1));
    printf("Test 1: %s\\n", buffer1);
    
    // Test case 2: Copy partial string
    char buffer2[10] = {0};
    const char* source2 = "Hello";
    copyBytes(buffer2, source2, 3);
    printf("Test 2: %s\\n", buffer2);
    
    // Test case 3: Copy to exact size buffer
    char buffer3[6] = {0};
    const char* source3 = "Hello";
    copyBytes(buffer3, source3, 5);
    printf("Test 3: %s\\n", buffer3);
    
    // Test case 4: Copy empty string
    char buffer4[10] = {0};
    const char* source4 = "";
    copyBytes(buffer4, source4, 0);
    printf("Test 4: (empty)\\n");
    
    // Test case 5: Copy another string
    char buffer5[15] = {0};
    const char* source5 = "World";
    copyBytes(buffer5, source5, strlen(source5));
    printf("Test 5: %s\\n", buffer5);
    
    return 0;
}
