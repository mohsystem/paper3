
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/**
 * Safely copies a specified number of bytes from source to destination.
 * 
 * @param dest Destination buffer pointer
 * @param src Source buffer pointer
 * @param numBytes Number of bytes to copy
 * @param destSize Size of destination buffer (for bounds checking)
 * @param srcSize Size of source buffer (for bounds checking)
 * @return Number of bytes actually copied, -1 on error
 */
int safeCopy(void* dest, const void* src, size_t numBytes, size_t destSize, size_t srcSize) {
    /* Input validation: check for null pointers */
    if (dest == NULL || src == NULL) {
        return -1;
    }
    
    /* Bounds checking: ensure numBytes doesn't exceed source buffer size */\n    if (numBytes > srcSize) {\n        return -1;\n    }\n    \n    /* Bounds checking: ensure numBytes doesn't exceed destination buffer size */
    if (numBytes > destSize) {
        return -1;
    }
    
    /* Check for integer overflow (numBytes is size_t so already unsigned) */
    if (numBytes > SIZE_MAX) {
        return -1;
    }
    
    /* Safe copy using memcpy with validated bounds */
    memcpy(dest, src, numBytes);
    
    return (int)numBytes;
}

int main(void) {
    /* Test case 1: Normal copy */
    char dest1[20] = {0};
    const char* src1 = "Hello";
    int result1 = safeCopy(dest1, src1, strlen(src1), sizeof(dest1), strlen(src1));
    printf("Test 1: %s\\n", result1 == (int)strlen(src1) ? "PASS" : "FAIL");
    
    /* Test case 2: Copy with buffer larger than source */
    char dest2[100] = {0};
    const char* src2 = "Test";
    int result2 = safeCopy(dest2, src2, strlen(src2), sizeof(dest2), strlen(src2));
    printf("Test 2: %s\\n", result2 == (int)strlen(src2) ? "PASS" : "FAIL");
    
    /* Test case 3: Attempt to copy more bytes than source has (should fail) */
    char dest3[20] = {0};
    const char* src3 = "Hi";
    int result3 = safeCopy(dest3, src3, 10, sizeof(dest3), strlen(src3));
    printf("Test 3: %s\\n", result3 == -1 ? "PASS" : "FAIL");
    
    /* Test case 4: Attempt to copy more bytes than destination can hold (should fail) */
    char dest4[3] = {0};
    const char* src4 = "Hello World";
    int result4 = safeCopy(dest4, src4, strlen(src4), sizeof(dest4), strlen(src4));
    printf("Test 4: %s\\n", result4 == -1 ? "PASS" : "FAIL");
    
    /* Test case 5: Null pointer check */
    char dest5[10] = {0};
    int result5 = safeCopy(dest5, NULL, 5, sizeof(dest5), 5);
    printf("Test 5: %s\\n", result5 == -1 ? "PASS" : "FAIL");
    
    return 0;
}
