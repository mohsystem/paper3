
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Safely copies bytes from source to destination
 * @param dest Destination buffer
 * @param destSize Size of destination buffer
 * @param src Source buffer
 * @param srcSize Size of source buffer
 * @param numBytes Number of bytes to copy
 * @return true if successful, false otherwise
 */
bool safeCopyBytes(void* dest, size_t destSize, const void* src, size_t srcSize, size_t numBytes) {
    // Input validation
    if (dest == NULL || src == NULL) {
        fprintf(stderr, "Error: NULL pointer provided\\n");
        return false;
    }
    
    if (numBytes == 0) {
        return true; // Nothing to copy
    }
    
    // Bounds checking
    if (numBytes > srcSize) {
        fprintf(stderr, "Error: Source buffer too small\\n");
        return false;
    }
    
    if (numBytes > destSize) {
        fprintf(stderr, "Error: Destination buffer too small\\n");
        return false;
    }
    
    // Check for overflow
    if (numBytes > SIZE_MAX) {
        fprintf(stderr, "Error: Byte count overflow\\n");
        return false;
    }
    
    // Safe copy operation using memmove (handles overlapping memory)
    memmove(dest, src, numBytes);
    return true;
}

void printBuffer(const unsigned char* buffer, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; i++) {
        printf("%d", buffer[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    printf("=== Test Case 1: Normal copy ===\\n");
    unsigned char src1[] = {1, 2, 3, 4, 5};
    unsigned char dest1[5] = {0};
    bool result1 = safeCopyBytes(dest1, sizeof(dest1), src1, sizeof(src1), 5);
    printf("Result: %s, Dest: ", result1 ? "true" : "false");
    printBuffer(dest1, sizeof(dest1));
    
    printf("\\n=== Test Case 2: Partial copy ===\\n");
    unsigned char src2[] = {10, 20, 30, 40, 50};
    unsigned char dest2[10] = {0};
    bool result2 = safeCopyBytes(dest2, sizeof(dest2), src2, sizeof(src2), 3);
    printf("Result: %s, Dest: ", result2 ? "true" : "false");
    printBuffer(dest2, sizeof(dest2));
    
    printf("\\n=== Test Case 3: NULL source ===\\n");
    unsigned char dest3[5] = {0};
    bool result3 = safeCopyBytes(dest3, sizeof(dest3), NULL, 0, 5);
    printf("Result: %s\\n", result3 ? "true" : "false");
    
    printf("\\n=== Test Case 4: Buffer overflow attempt ===\\n");
    unsigned char src4[] = {1, 2, 3};
    unsigned char dest4[2] = {0};
    bool result4 = safeCopyBytes(dest4, sizeof(dest4), src4, sizeof(src4), 3);
    printf("Result: %s\\n", result4 ? "true" : "false");
    
    printf("\\n=== Test Case 5: Zero bytes ===\\n");
    unsigned char src5[] = {1, 2, 3};
    unsigned char dest5[3] = {0};
    bool result5 = safeCopyBytes(dest5, sizeof(dest5), src5, sizeof(src5), 0);
    printf("Result: %s, Dest: ", result5 ? "true" : "false");
    printBuffer(dest5, sizeof(dest5));
    
    return 0;
}
