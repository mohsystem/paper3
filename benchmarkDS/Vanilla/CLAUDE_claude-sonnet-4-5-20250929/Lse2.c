
#include <stdio.h>
#include <string.h>

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
    printf("Test 1: %s\\n", dest1);
    
    // Test case 2: Copy fewer bytes than source
    char dest2[20] = {0};
    const char* src2 = "Hello World";
    copyBytes(dest2, src2, 5);
    printf("Test 2: %s\\n", dest2);
    
    // Test case 3: Empty source
    char dest3[20] = {0};
    const char* src3 = "";
    copyBytes(dest3, src3, 0);
    printf("Test 3: %s\\n", dest3);
    
    // Test case 4: Copy with exact buffer size
    char dest4[6] = {0};
    const char* src4 = "12345";
    copyBytes(dest4, src4, 5);
    printf("Test 4: %s\\n", dest4);
    
    // Test case 5: Larger numBytes than source
    char dest5[20] = {0};
    const char* src5 = "Test";
    copyBytes(dest5, src5, strlen(src5));
    printf("Test 5: %s\\n", dest5);
    
    return 0;
}
