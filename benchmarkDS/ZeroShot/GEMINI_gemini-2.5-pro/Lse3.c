#include <stdio.h>
#include <string.h> // For memmove, strlen, memset
#include <stdlib.h> // For NULL

/**
 * Securely copies a number of bytes from one memory location to another.
 *
 * @param dest Pointer to the destination buffer.
 * @param dest_size The maximum capacity of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param num_bytes The number of bytes to copy from src to dest.
 * @return 0 on success, and a non-zero value on failure.
 */
int secure_copy_bytes(void* dest, size_t dest_size, const void* src, size_t num_bytes) {
    // 1. Check for null pointers
    if (dest == NULL || src == NULL) {
        fprintf(stderr, "Error: Destination or source pointer is null.\n");
        return 1; // Failure code for null pointer
    }

    // 2. A zero-byte copy is a no-op and is considered a success.
    if (num_bytes == 0) {
        return 0; // Success
    }

    // 3. Check for buffer overflow. This is the most critical security check.
    if (num_bytes > dest_size) {
        fprintf(stderr, "Error: Buffer overflow detected. Cannot copy %zu bytes into a destination of size %zu.\n", num_bytes, dest_size);
        return 2; // Failure code for overflow
    }

    // 4. Use memmove for the copy. It's safer than memcpy as it correctly
    // handles potentially overlapping memory regions.
    memmove(dest, src, num_bytes);
    
    return 0; // Success
}

void run_tests() {
    printf("--- C Test Cases ---\n");

    const char* source_string = "Hello";
    // +1 for the null terminator to copy the C-style string properly
    size_t bytes_to_copy = strlen(source_string) + 1; 

    // Test Case 1: Normal copy
    printf("\n--- Test Case 1: Normal copy ---\n");
    char dest1[10];
    memset(dest1, 0, sizeof(dest1)); // Clear buffer
    if (secure_copy_bytes(dest1, sizeof(dest1), source_string, bytes_to_copy) == 0) {
        printf("Success! Copied string: \"%s\"\n", dest1);
    } else {
        printf("Failed!\n");
    }

    // Test Case 2: Exact size copy
    printf("\n--- Test Case 2: Exact size copy ---\n");
    char dest2[6];
    memset(dest2, 0, sizeof(dest2));
    if (secure_copy_bytes(dest2, sizeof(dest2), source_string, bytes_to_copy) == 0) {
        printf("Success! Copied string: \"%s\"\n", dest2);
    } else {
        printf("Failed!\n");
    }

    // Test Case 3: Buffer overflow attempt
    printf("\n--- Test Case 3: Buffer overflow attempt ---\n");
    char dest3[4];
    memset(dest3, 0, sizeof(dest3));
    if (secure_copy_bytes(dest3, sizeof(dest3), source_string, bytes_to_copy) == 0) {
        printf("Success! (This should not happen)\n");
    } else {
        printf("Failed as expected.\n");
    }

    // Test Case 4: Null pointer
    printf("\n--- Test Case 4: Null pointer ---\n");
    char* dest4 = NULL;
    if (secure_copy_bytes(dest4, 10, source_string, bytes_to_copy) == 0) {
        printf("Success! (This should not happen)\n");
    } else {
        printf("Failed as expected.\n");
    }

    // Test Case 5: Zero bytes copy
    printf("\n--- Test Case 5: Zero bytes copy ---\n");
    char dest5[10];
    memset(dest5, 0, sizeof(dest5));
    if (secure_copy_bytes(dest5, sizeof(dest5), source_string, 0) == 0) {
        printf("Success! Copied 0 bytes.\n");
    } else {
        printf("Failed! (This should not happen)\n");
    }
}


int main() {
    run_tests();
    return 0;
}