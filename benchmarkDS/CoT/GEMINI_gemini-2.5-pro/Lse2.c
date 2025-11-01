#include <stdio.h>
#include <string.h> // For strlen in tests
#include <stdlib.h> // For size_t

/**
 * @brief Copies n bytes from memory area src to memory area dest.
 * @param dest A pointer to the destination memory area.
 * @param src A pointer to the source memory area.
 * @param n The number of bytes to be copied.
 * @note The memory areas must not overlap. For overlapping memory, memmove should be used.
 */
void my_memcpy(void* dest, const void* src, size_t n) {
    // 1. Security Check: Handle NULL pointers to prevent segmentation faults.
    if (dest == NULL || src == NULL) {
        fprintf(stderr, "Error: Destination or source pointer is NULL.\n");
        return;
    }
    
    // Cast void pointers to char pointers for byte-level manipulation.
    // A char is guaranteed to be 1 byte.
    char* d = (char*)dest;
    const char* s = (const char*)src;

    // 2. Perform the copy operation.
    // The loop is a secure way to copy as long as the caller ensures
    // that 'n' does not exceed the allocated sizes of 'dest' and 'src'.
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
}

void run_tests() {
    // --- Test Cases ---

    // Test Case 1: Standard copy of a string.
    printf("--- Test Case 1: Standard copy ---\n");
    const char* src1 = "Hello C";
    char dest1[20];
    memset(dest1, 0, 20); // Initialize with zeros
    size_t n1 = strlen(src1) + 1; // +1 for null terminator
    printf("Before copy: dest1 = \"%s\"\n", dest1);
    my_memcpy(dest1, src1, n1);
    printf("After copy:  dest1 = \"%s\"\n\n", dest1);

    // Test Case 2: Partial copy of an integer array.
    printf("--- Test Case 2: Partial copy ---\n");
    int src2[] = {10, 20, 30, 40, 50};
    int dest2[10];
    memset(dest2, 0, sizeof(dest2));
    size_t n2 = 3 * sizeof(int); // Copy first 3 integers
    printf("Before copy: dest2[0] = %d, dest2[2] = %d\n", dest2[0], dest2[2]);
    my_memcpy(dest2, src2, n2);
    printf("After copy:  dest2[0] = %d, dest2[2] = %d\n\n", dest2[0], dest2[2]);

    // Test Case 3: Zero-byte copy.
    printf("--- Test Case 3: Zero-byte copy ---\n");
    const char* src3 = "Source";
    char dest3[10] = "Original";
    size_t n3 = 0;
    printf("Before copy: dest3 = \"%s\"\n", dest3);
    my_memcpy(dest3, src3, n3);
    printf("After copy:  dest3 = \"%s\"\n\n", dest3);

    // Test Case 4: Copying into a buffer that is smaller than the source, but copy size is safe.
    printf("--- Test Case 4: Safe copy to smaller buffer ---\n");
    const char* src4 = "This is a long source string";
    char dest4[6];
    memset(dest4, 0, 6);
    size_t n4 = 5;
    printf("Before copy: dest4 = \"%s\"\n", dest4);
    my_memcpy(dest4, src4, n4);
    // dest4[5] is already '\0' from memset, ensuring it's a valid string for printing
    printf("After copy:  dest4 = \"%s\"\n\n", dest4);

    // Test Case 5: Boundary Check (caller's responsibility).
    // In C, it's the programmer's job to prevent overflows before calling the function.
    printf("--- Test Case 5: Boundary check (caller's responsibility) ---\n");
    char src5[] = {'a', 'b', 'c', 'd', 'e'};
    char dest5[3];
    size_t n5 = sizeof(src5);
    size_t dest5_size = sizeof(dest5);
    printf("Attempting to copy %zu bytes into a %zu-byte buffer.\n", n5, dest5_size);
    if (n5 > dest5_size) {
        printf("Error: Copy size is larger than destination buffer. Aborting copy.\n");
    } else {
        my_memcpy(dest5, src5, n5);
    }
    printf("\n");
}


// In C, there is no main class. The main function serves as the entry point.
int main() {
    run_tests();
    return 0;
}