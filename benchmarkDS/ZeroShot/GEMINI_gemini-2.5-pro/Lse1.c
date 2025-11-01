#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Securely copies a number of bytes from a source to a destination buffer.
 * 
 * @param dest Pointer to the destination buffer.
 * @param dest_size The total allocated size of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param n The number of bytes to copy.
 * @return A pointer to the destination buffer on success, NULL on failure.
 */
void* secure_copy(void* dest, size_t dest_size, const void* src, size_t n) {
    if (dest == NULL || src == NULL) {
        fprintf(stderr, "ERROR: Source or destination pointer cannot be NULL.\n");
        return NULL;
    }
    if (n > dest_size) {
        fprintf(stderr, "ERROR: Destination buffer overflow detected. Copy aborted.\n");
        return NULL;
    }
    if (n == 0) {
        return dest; // Nothing to copy, operation is trivially successful.
    }
    
    // memmove is safe for overlapping memory regions.
    return memmove(dest, src, n);
}

// Helper function to print a buffer
void print_buffer(const char* name, const char* buffer, size_t size) {
    printf("%s: [", name);
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", buffer[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

int main() {
    // Test Case 1: Normal successful copy
    printf("--- Test Case 1: Normal Copy ---\n");
    char* src1 = (char*)malloc(7);
    memcpy(src1, (char[]){1, 2, 3, 4, 5, 6, 7}, 7);
    char* dest1 = (char*)malloc(5);
    memset(dest1, 0, 5);
    print_buffer("Destination before copy", dest1, 5);
    if (secure_copy(dest1, 5, src1, 5) != NULL) {
        print_buffer("Destination after copy ", dest1, 5);
        printf("Expected: [1, 2, 3, 4, 5]\n");
    }
    free(src1);
    free(dest1);
    printf("\n");

    // Test Case 2: Destination buffer too small (potential overflow)
    printf("--- Test Case 2: Destination Too Small ---\n");
    char* src2 = (char*)malloc(5);
    memcpy(src2, (char[]){10, 20, 30, 40, 50}, 5);
    char* dest2 = (char*)malloc(3);
    printf("Attempting to copy 5 bytes into a 3-byte array.\n");
    if (secure_copy(dest2, 3, src2, 5) == NULL) {
        printf("Caught expected error.\n");
    }
    free(src2);
    free(dest2);
    printf("\n");

    // Test Case 3: Copying zero bytes
    printf("--- Test Case 3: Zero Bytes to Copy ---\n");
    char* src3 = (char*)malloc(5);
    memcpy(src3, (char[]){1, 1, 1, 1, 1}, 5);
    char* dest3 = (char*)malloc(5);
    memcpy(dest3, (char[]){9, 9, 9, 9, 9}, 5);
    print_buffer("Destination before copy", dest3, 5);
    if (secure_copy(dest3, 5, src3, 0) != NULL) {
        print_buffer("Destination after copy ", dest3, 5);
        printf("Expected: [9, 9, 9, 9, 9]\n");
    }
    free(src3);
    free(dest3);
    printf("\n");

    // Test Case 4: Overlapping memory regions
    printf("--- Test Case 4: Overlapping Memory ---\n");
    char* buffer4 = (char*)malloc(10);
    memcpy(buffer4, (char[]){1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 10);
    print_buffer("Buffer before copy", buffer4, 10);
    char* src_ptr = buffer4;
    char* dest_ptr = buffer4 + 2;
    if (secure_copy(dest_ptr, 10 - 2, src_ptr, 5) != NULL) {
        print_buffer("Buffer after copy ", buffer4, 10);
        printf("Expected: [1, 2, 1, 2, 3, 4, 5, 8, 9, 10]\n");
    }
    free(buffer4);
    printf("\n");

    // Test Case 5: Null pointer input
    printf("--- Test Case 5: Null Input ---\n");
    char* src5 = (char*)malloc(3);
    memcpy(src5, (char[]){1, 2, 3}, 3);
    printf("Attempting to copy to a NULL destination.\n");
    if (secure_copy(NULL, 10, src5, 3) == NULL) {
        printf("Caught expected error.\n");
    }
    free(src5);
    printf("\n");

    return 0;
}