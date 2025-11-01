
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Securely copies bytes from source to destination with bounds checking
 * @param dest destination buffer
 * @param destSize size of destination buffer
 * @param src source buffer
 * @param srcSize size of source buffer
 * @param numBytes number of bytes to copy
 * @return true if successful, false otherwise
 */
bool secureCopy(char* dest, size_t destSize, const char* src, size_t srcSize, size_t numBytes) {
    // Null pointer checks
    if (dest == NULL || src == NULL) {
        printf("Error: Null buffer provided\\n");
        return false;
    }
    
    // Check if source has enough bytes
    if (numBytes > srcSize) {
        printf("Error: Source buffer too small\\n");
        return false;
    }
    
    // Check if destination has enough space
    if (numBytes > destSize) {
        printf("Error: Destination buffer too small\\n");
        return false;
    }
    
    // Perform the copy using safe function
    memcpy(dest, src, numBytes);
    return true;
}

int main() {
    printf("=== Test Case 1: Copy 'Hello' ===\\n");
    char dest1[10] = {0};
    const char* src1 = "Hello";
    if (secureCopy(dest1, sizeof(dest1), src1, strlen(src1), strlen(src1))) {
        printf("Result: %s\\n", dest1);
    }
    
    printf("\\n=== Test Case 2: Buffer too small ===\\n");
    char dest2[3] = {0};
    const char* src2 = "Hello";
    secureCopy(dest2, sizeof(dest2), src2, strlen(src2), strlen(src2));
    
    printf("\\n=== Test Case 3: Zero bytes ===\\n");
    char dest3[10] = {0};
    const char* src3 = "Test";
    if (secureCopy(dest3, sizeof(dest3), src3, strlen(src3), 0)) {
        printf("Result: Successfully copied 0 bytes\\n");
    }
    
    printf("\\n=== Test Case 4: Null source ===\\n");
    char dest4[10] = {0};
    secureCopy(dest4, sizeof(dest4), NULL, 0, 5);
    
    printf("\\n=== Test Case 5: Exact size match ===\\n");
    char dest5[6] = {0};
    const char* src5 = "World";
    if (secureCopy(dest5, sizeof(dest5) - 1, src5, strlen(src5), strlen(src5))) {
        printf("Result: %s\\n", dest5);
    }
    
    return 0;
}
