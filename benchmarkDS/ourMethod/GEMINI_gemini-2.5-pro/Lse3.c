#include <stdio.h>
#include <string.h> // For strlen, memset, strcmp
#include <stddef.h> // For size_t
#include <stdlib.h> // For NULL

/**
 * Copies a number of bytes from one memory location to another.
 * This implementation does not handle overlapping memory regions.
 * For overlapping regions, a function like memmove should be used.
 *
 * @param dest Pointer to the destination memory. Must not be NULL.
 * @param src  Pointer to the source memory. Must not be NULL.
 * @param n    The number of bytes to copy.
 * @return A pointer to the destination memory (dest), or NULL on error.
 */
void* customMemcpy(void* dest, const void* src, size_t n) {
    // Rule #4: Input validation
    if (dest == NULL || src == NULL) {
        return NULL;
    }

    // Cast pointers to operate on bytes
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    // Rule #1: Ensure that buffer operations adhere to boundaries.
    // The loop iterates exactly 'n' times, copying one byte at a time.
    // The CALLER is responsible for ensuring 'dest' and 'src' point to
    // valid memory regions of at least 'n' bytes. This function cannot
    // verify the allocated sizes of the passed-in buffers.
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

void run_test_cases() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Copy "Hello" including null terminator
    printf("\n--- Test Case 1: Copy 'Hello' ---\n");
    const char* src1 = "Hello";
    char dest1[10]; // Rule #2: Ensure buffer size is sufficient
    // Copy string length + 1 for the null terminator '\0'
    size_t bytesToCopy1 = strlen(src1) + 1;
    
    printf("Source: \"%s\"\n", src1);
    memset(dest1, 0, sizeof(dest1)); // Initialize destination buffer
    if (customMemcpy(dest1, src1, bytesToCopy1) != NULL) {
        printf("Destination (after):  \"%s\"\n", dest1);
        printf("strcmp result: %s\n", strcmp(src1, dest1) == 0 ? "Match" : "Mismatch");
    }

    // Test Case 2: Copy with n = 0
    printf("\n--- Test Case 2: Copy 0 bytes ---\n");
    const char* src2 = "Test";
    char dest2[10] = "Original";
    printf("Destination (before): \"%s\"\n", dest2);
    customMemcpy(dest2, src2, 0);
    printf("Destination (after):  \"%s\"\n", dest2);

    // Test Case 3: Copy part of a longer string
    printf("\n--- Test Case 3: Copy part of a string ---\n");
    const char* src3 = "World12345";
    char dest3[6];
    memset(dest3, 0, sizeof(dest3));
    customMemcpy(dest3, src3, 5); // Copy "World"
    // dest3[5] is already '\0' from memset, but it is good practice to be explicit
    printf("Copied part: \"%s\"\n", dest3);

    // Test Case 4: Copy from one buffer to another
    printf("\n--- Test Case 4: Copy from array to array ---\n");
    char src4[] = {'A', 'B', 'C', 'D', '\0'};
    char dest4[10] = {0};
    printf("Source data: \"%s\"\n", src4);
    customMemcpy(dest4, src4, sizeof(src4));
    printf("Destination data: \"%s\"\n", dest4);

    // Test Case 5: Null pointer input
    printf("\n--- Test Case 5: Null pointer input ---\n");
    char dest5[10];
    if (customMemcpy(NULL, "test", 4) == NULL) {
        printf("Handled NULL for destination correctly.\n");
    }
    if (customMemcpy(dest5, NULL, 4) == NULL) {
        printf("Handled NULL for source correctly.\n");
    }
}

int main() {
    run_test_cases();
    return 0;
}