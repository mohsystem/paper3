
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Copies bytes from source to destination with bounds checking.
 * @param dest Destination buffer
 * @param dest_size Size of destination buffer
 * @param src Source buffer
 * @param src_size Size of source buffer
 * @param num_bytes Number of bytes to copy
 * @return true if copy successful, false otherwise
 */
bool safe_copy(void* dest, size_t dest_size, const void* src, size_t src_size, size_t num_bytes) {
    /* Validate all pointers are non-NULL to prevent NULL pointer dereference */
    if (dest == NULL || src == NULL) {
        return false;
    }
    
    /* Validate num_bytes does not exceed source size to prevent out-of-bounds read */
    if (num_bytes > src_size) {
        return false;
    }
    
    /* Validate num_bytes does not exceed destination size to prevent buffer overflow */
    if (num_bytes > dest_size) {
        return false;
    }
    
    /* Check for integer overflow in size calculations */
    if (num_bytes > SIZE_MAX) {
        return false;
    }
    
    /* Perform safe copy using memcpy with validated bounds */
    memcpy(dest, src, num_bytes);
    return true;
}

int main(void) {
    /* Test case 1: Normal copy of "Hello" */
    char buffer1[10] = {0};
    const char* hello = "Hello";
    size_t hello_len = strlen(hello);
    if (safe_copy(buffer1, sizeof(buffer1), hello, hello_len + 1, hello_len)) {
        printf("Test 1 passed: %s\\n", buffer1);
    }
    
    /* Test case 2: Destination buffer too small (should fail safely) */
    char buffer2[3] = {0};
    if (!safe_copy(buffer2, sizeof(buffer2), hello, hello_len + 1, hello_len)) {
        printf("Test 2 passed: Correctly rejected overflow\\n");
    }
    
    /* Test case 3: Copy zero bytes (edge case) */
    char buffer3[10] = {0};
    if (safe_copy(buffer3, sizeof(buffer3), hello, hello_len + 1, 0)) {
        printf("Test 3 passed: Zero byte copy succeeded\\n");
    }
    
    /* Test case 4: NULL destination (should fail safely) */
    if (!safe_copy(NULL, 10, hello, hello_len + 1, hello_len)) {
        printf("Test 4 passed: Correctly rejected NULL destination\\n");
    }
    
    /* Test case 5: Copy exceeds source size (should fail safely) */
    char buffer5[10] = {0};
    if (!safe_copy(buffer5, sizeof(buffer5), hello, hello_len + 1, 100)) {
        printf("Test 5 passed: Correctly rejected excessive count\\n");
    }
    
    return 0;
}
