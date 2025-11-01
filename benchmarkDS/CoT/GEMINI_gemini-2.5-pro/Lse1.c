#include <stdio.h>
#include <stdlib.h> // For NULL, size_t, malloc, free
#include <string.h> // For strcmp

/**
 * Copies a number of bytes from one memory location to another.
 * Assumes non-overlapping memory regions for safety and performance,
 * similar to the standard C memcpy.
 *
 * @param destination Pointer to the destination memory block.
 * @param source      Pointer to the source memory block.
 * @param numBytes    Number of bytes to copy.
 * @return A pointer to the destination.
 */
void* memoryCopy(void* destination, const void* source, size_t numBytes) {
    // 1. Security Check: Prevent Null Pointer Dereference.
    if (destination == NULL || source == NULL) {
        return NULL;
    }

    // 2. Check for no-op.
    if (numBytes == 0) {
        return destination;
    }
    
    // 3. Cast pointers to char* to perform byte-by-byte operations.
    char* dest_ptr = (char*)destination;
    const char* src_ptr = (const char*)source;

    // Caller's Responsibility: The caller must ensure that source and destination
    // buffers are large enough to hold numBytes. C cannot know the
    // allocated size of a raw pointer, so bounds checking inside this function
    // is not possible. This is a primary source of vulnerabilities in C.
    
    // 4. Core Logic: Perform the byte-by-byte copy.
    for (size_t i = 0; i < numBytes; i++) {
        dest_ptr[i] = src_ptr[i];
    }
    
    return destination;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Normal copy
    const char* src1 = "Hello World";
    char* dest1 = (char*)malloc(strlen(src1) + 1);
    if (dest1) {
        memoryCopy(dest1, src1, strlen(src1) + 1); // +1 for null terminator
        printf("Test 1 (Normal): %s\n", dest1);
        free(dest1);
    }

    // Test Case 2: Partial copy
    const char* src2 = "Programming";
    char* dest2 = (char*)malloc(20);
    if (dest2) {
        memset(dest2, 0, 20); // Clear buffer
        memoryCopy(dest2, src2, 7);
        printf("Test 2 (Partial): %s\n", dest2);
        free(dest2);
    }
    
    // Test Case 3: Zero-byte copy
    const char* src3 = "Test";
    char dest3[10] = "Original";
    memoryCopy(dest3, src3, 0);
    printf("Test 3 (Zero bytes): %s\n", strcmp(dest3, "Original") == 0 ? "Unchanged" : "Changed");

    // Test Case 4: Null input (should not crash)
    const char* src4 = "Test";
    void* result4 = memoryCopy(NULL, src4, 4);
    if (result4 == NULL) {
        printf("Test 4 (Null input): Handled gracefully\n");
    } else {
        printf("Test 4 (Null input): Failed\n");
    }

    // Test Case 5: Copy to self (pointers are identical)
    char buffer5[] = "abcdef";
    memoryCopy(buffer5, buffer5, 3);
    printf("Test 5 (Copy to self): %s\n", strcmp(buffer5, "abcdef") == 0 ? "Unchanged" : "Changed");

    return 0;
}