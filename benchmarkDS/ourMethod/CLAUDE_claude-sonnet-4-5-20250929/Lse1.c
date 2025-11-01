
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * Copies a specified number of bytes from source to destination.
 * 
 * @param destination Pointer to destination buffer
 * @param source Pointer to source buffer
 * @param numBytes Number of bytes to copy
 * @return true if copy successful, false otherwise
 */
bool copyBytes(void* destination, const void* source, size_t numBytes) {
    /* Input validation: check for null pointers */
    if (destination == NULL || source == NULL) {
        return false;
    }
    
    /* Input validation: check for zero bytes (valid but no-op) */
    if (numBytes == 0) {
        return true;
    }
    
    /* Prevent potential integer overflow in pointer arithmetic */
    if (numBytes > SIZE_MAX) {
        return false;
    }
    
    /* Additional safety: check for pointer overlap */
    const unsigned char* src_ptr = (const unsigned char*)source;
    unsigned char* dest_ptr = (unsigned char*)destination;
    
    /* Check for overlapping regions - use memmove if overlap detected */
    if ((dest_ptr >= src_ptr && dest_ptr < src_ptr + numBytes) ||
        (src_ptr >= dest_ptr && src_ptr < dest_ptr + numBytes)) {
        memmove(destination, source, numBytes);
        return true;
    }
    
    /* Safe copy operation - memcpy does not perform bounds checking */
    /* Caller must ensure buffers are large enough */
    memcpy(destination, source, numBytes);
    return true;
}

int main(void) {
    /* Test case 1: Normal copy */
    unsigned char dest1[10] = {0};
    unsigned char src1[] = {1, 2, 3, 4, 5};
    bool result1 = copyBytes(dest1, src1, 5);
    printf("Test 1 - Normal copy: %d, Dest: ", result1);
    for (int i = 0; i < 10; i++) printf("%d ", dest1[i]);
    printf("\\n");
    
    /* Test case 2: Copy with numBytes = 0 */
    unsigned char dest2[5] = {0};
    unsigned char src2[] = {10, 20, 30};
    bool result2 = copyBytes(dest2, src2, 0);
    printf("Test 2 - Zero bytes: %d\\n", result2);
    
    /* Test case 3: Overlapping regions (handled by memmove) */
    unsigned char buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    bool result3 = copyBytes(buffer + 2, buffer, 5);
    printf("Test 3 - Overlapping copy: %d, Buffer: ", result3);
    for (int i = 0; i < 10; i++) printf("%d ", buffer[i]);
    printf("\\n");
    
    /* Test case 4: Null source (should fail) */
    unsigned char dest4[5] = {0};
    bool result4 = copyBytes(dest4, NULL, 3);
    printf("Test 4 - Null source: %d\\n", result4);
    
    /* Test case 5: Null destination (should fail) */
    unsigned char src5[] = {1, 2, 3};
    bool result5 = copyBytes(NULL, src5, 3);
    printf("Test 5 - Null destination: %d\\n", result5);
    
    return 0;
}
