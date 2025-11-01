#include <stdio.h>
#include <string.h> // For strlen
#include <ctype.h>  // For isprint

/**
 * @brief Copies n bytes from memory area src to memory area dest.
 * 
 * @param dest Pointer to the destination array where the content is to be copied.
 * @param src Pointer to the source of data to be copied.
 * @param n The number of bytes to copy.
 * @return A pointer to the destination.
 */
void* memoryCopy(void* dest, const void* src, size_t n) {
    // Cast the void pointers to char pointers for byte-wise operations
    char* cdest = (char*)dest;
    const char* csrc = (const char*)src;

    for (size_t i = 0; i < n; i++) {
        cdest[i] = csrc[i];
    }

    return dest;
}

int main() {
    // Test Case 1: Copy "Hello"
    printf("--- Test Case 1: Copy 'Hello' ---\n");
    const char* src1 = "Hello";
    char dest1[10] = {0}; // Initialize with null bytes
    memoryCopy(dest1, src1, strlen(src1) + 1); // +1 for null terminator
    printf("Source: %s\n", src1);
    printf("Destination: %s\n", dest1);
    printf("\n");

    // Test Case 2: Copy a longer string
    printf("--- Test Case 2: Copy a longer string ---\n");
    const char* src2 = "This is a test string.";
    char dest2[30] = {0};
    memoryCopy(dest2, src2, strlen(src2) + 1);
    printf("Source: %s\n", src2);
    printf("Destination: %s\n", dest2);
    printf("\n");

    // Test Case 3: Copy a portion of a string
    printf("--- Test Case 3: Copy a portion ---\n");
    const char* src3 = "Programming";
    char dest3[15] = {0};
    memoryCopy(dest3, src3, 7); // Copy "Program"
    dest3[7] = '\0'; // Manually add null terminator
    printf("Source: %s\n", src3);
    printf("Destination (first 7 bytes): %s\n", dest3);
    printf("\n");
    
    // Test Case 4: Copy an array of integers
    printf("--- Test Case 4: Copy integer array ---\n");
    int src4[] = {100, 200, 300, 400};
    int dest4[4] = {0};
    memoryCopy(dest4, src4, sizeof(src4));
    printf("Source integers: %d %d %d %d\n", src4[0], src4[1], src4[2], src4[3]);
    printf("Destination integers: %d %d %d %d\n", dest4[0], dest4[1], dest4[2], dest4[3]);
    printf("\n");

    // Test Case 5: Copy zero bytes
    printf("--- Test Case 5: Copy zero bytes ---\n");
    const char* src5 = "Should not be copied";
    char dest5[20];
    // Fill destination to see the effect
    memset(dest5, 'X', 19);
    dest5[19] = '\0';
    memoryCopy(dest5, src5, 0);
    printf("Source: %s\n", src5);
    printf("Destination after copying 0 bytes: %s\n", dest5);
    printf("\n");

    return 0;
}