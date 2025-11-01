
#include <stdio.h>
#include <string.h>
#include <stddef.h>

void copyBytes(unsigned char* destination, const unsigned char* source, size_t numBytes) {
    if (destination == NULL || source == NULL) {
        fprintf(stderr, "Error: Destination and source cannot be null\\n");
        return;
    }
    
    for (size_t i = 0; i < numBytes; i++) {
        destination[i] = source[i];
    }
}

int main() {
    // Test case 1: Normal copy
    unsigned char dest1[20] = {0};
    const unsigned char* src1 = (const unsigned char*)"Hello World";
    copyBytes(dest1, src1, 11);
    printf("Test 1: %s\\n", dest1);
    
    // Test case 2: Partial copy
    unsigned char dest2[10] = {0};
    const unsigned char* src2 = (const unsigned char*)"Testing";
    copyBytes(dest2, src2, 4);
    printf("Test 2: %s\\n", dest2);
    
    // Test case 3: Copy with exact size
    unsigned char dest3[6] = {0};
    const unsigned char* src3 = (const unsigned char*)"Exact";
    copyBytes(dest3, src3, 5);
    printf("Test 3: %s\\n", dest3);
    
    // Test case 4: Copy zero bytes
    unsigned char dest4[10] = {0};
    const unsigned char* src4 = (const unsigned char*)"Zero";
    copyBytes(dest4, src4, 0);
    printf("Test 4: Empty copy successful\\n");
    
    // Test case 5: Copy single byte
    unsigned char dest5[10] = {0};
    const unsigned char* src5 = (const unsigned char*)"A";
    copyBytes(dest5, src5, 1);
    printf("Test 5: %s\\n", dest5);
    
    return 0;
}
