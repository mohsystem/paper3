#include <stdio.h>
#include <string.h> // For strlen

/**
 * Copies n bytes from memory area src to memory area dest.
 *
 * @param dest Pointer to the destination array.
 * @param src  Pointer to the source array.
 * @param n    Number of bytes to be copied.
 * @return A pointer to the destination.
 */
void* my_memcpy(void* dest, const void* src, size_t n) {
    // Security Check: Ensure pointers are not null.
    if (dest == NULL || src == NULL) {
        return NULL;
    }
    
    // Cast pointers to char* to perform byte-level operations.
    char* d = (char*)dest;
    const char* s = (const char*)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

int main() {
    printf("C Tests:\n");

    // Test Case 1: Standard copy "Hello"
    const char* src1 = "Hello";
    char dest1[10];
    memset(dest1, 0, sizeof(dest1)); // Zero-initialize
    size_t len1 = strlen(src1) + 1; // +1 for null terminator
    printf("\n--- Test 1: Standard copy ---\n");
    printf("Copying %zu bytes from \"%s\".\n", len1, src1);
    printf("Destination buffer size: %zu\n", sizeof(dest1));
    // Security check by caller: ensure copy size does not exceed destination size.
    if (len1 <= sizeof(dest1)) {
        my_memcpy(dest1, src1, len1);
        printf("Destination after copy: \"%s\"\n", dest1);
    }

    // Test Case 2: Full buffer copy
    const char* src2 = "12345";
    char dest2[6];
    memset(dest2, 0, sizeof(dest2));
    size_t len2 = strlen(src2) + 1; // 5 + 1 = 6
    printf("\n--- Test 2: Full buffer copy ---\n");
    printf("Copying %zu bytes from \"%s\".\n", len2, src2);
    printf("Destination buffer size: %zu\n", sizeof(dest2));
    if (len2 <= sizeof(dest2)) {
        my_memcpy(dest2, src2, len2);
        printf("Destination after copy: \"%s\"\n", dest2);
    }

    // Test Case 3: Partial copy
    const char* src3 = "World";
    char dest3[10];
    memset(dest3, 0, sizeof(dest3));
    printf("\n--- Test 3: Partial copy ---\n");
    printf("Copying 3 bytes from \"%s\".\n", src3);
    my_memcpy(dest3, src3, 3);
    printf("Destination after copy: \"%s\" (Note: not null-terminated by the copy)\n", dest3);
    
    // Test Case 4: Copy empty string
    const char* src4 = "";
    char dest4[5] = {'X','X','X','X','\0'};
    size_t len4 = strlen(src4) + 1; // 1 byte for null terminator
    printf("\n--- Test 4: Copy empty string ---\n");
    printf("Destination before: \"%s\"\n", dest4);
    if (len4 <= sizeof(dest4)) {
        my_memcpy(dest4, src4, len4);
        printf("Destination after copy: \"%s\"\n", dest4);
    }

    // Test Case 5: Array to array copy
    char src5[] = {'A', 'B', 'C', 'D', '\0'};
    char dest5[5];
    memset(dest5, 0, sizeof(dest5));
    printf("\n--- Test 5: Array to array copy ---\n");
    printf("Copying %zu bytes from a char array.\n", sizeof(src5));
    if (sizeof(src5) <= sizeof(dest5)) {
        my_memcpy(dest5, src5, sizeof(src5));
        printf("Destination after copy: \"%s\"\n", dest5);
    }

    return 0;
}