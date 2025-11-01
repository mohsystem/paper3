
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool secureCopy(void* destination, const void* source, size_t destSize, size_t numBytes) {
    // Input validation
    if (destination == NULL || source == NULL) {
        fprintf(stderr, "Error: Null pointer provided\\n");
        return false;
    }
    
    if (numBytes > destSize) {
        fprintf(stderr, "Error: Copy size exceeds destination capacity\\n");
        return false;
    }
    
    // Safe copy operation using memcpy
    memcpy(destination, source, numBytes);
    return true;
}

int main() {
    // Test case 1: Normal copy
    char dest1[20] = {0};
    const char* src1 = "Hello World";
    printf("Test 1: %d\\n", secureCopy(dest1, src1, sizeof(dest1), strlen(src1) + 1));
    printf("Result: %s\\n", dest1);
    
    // Test case 2: Partial copy
    char dest2[10] = {0};
    const char* src2 = "Test";
    printf("\\nTest 2: %d\\n", secureCopy(dest2, src2, sizeof(dest2), 4));
    printf("Result: %s\\n", dest2);
    
    // Test case 3: Buffer overflow prevention
    char dest3[5] = {0};
    const char* src3 = "Too long message";
    printf("\\nTest 3: %d\\n", secureCopy(dest3, src3, sizeof(dest3), strlen(src3) + 1));
    
    // Test case 4: Null source
    char dest4[10] = {0};
    printf("\\nTest 4: %d\\n", secureCopy(dest4, NULL, sizeof(dest4), 5));
    
    // Test case 5: Zero bytes
    char dest5[10] = {0};
    const char* src5 = "Data";
    printf("\\nTest 5: %d\\n", secureCopy(dest5, src5, sizeof(dest5), 0));
    
    return 0;
}
