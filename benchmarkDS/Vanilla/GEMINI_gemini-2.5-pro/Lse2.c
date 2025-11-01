#include <stdio.h>
#include <string.h> // For memcpy and memset

/**
 * Copies a number of bytes from one memory location to another.
 *
 * @param destination Pointer to the destination memory.
 * @param source      Pointer to the source memory.
 * @param numBytes    Number of bytes to copy.
 */
void memoryCopy(void* destination, const void* source, size_t numBytes) {
    if (destination != NULL && source != NULL) {
        memcpy(destination, source, numBytes);
    }
}

// Helper function to print a character buffer
void printBuffer(const char* label, const char* buffer, size_t size) {
    printf("%s", label);
    for (size_t i = 0; i < size; ++i) {
        // Print non-printable chars as '.'
        char c = buffer[i];
        if (c >= 32 && c <= 126) {
            putchar(c);
        } else {
            putchar('.');
        }
    }
    putchar('\n');
}

int main() {
    // Test Case 1: Basic copy
    printf("--- Test Case 1: Basic Copy ---\n");
    const char* src1 = "Hello World";
    char dest1[16];
    memset(dest1, 0, 16); // Clear destination buffer
    printBuffer("Destination before copy: ", dest1, 15);
    memoryCopy(dest1, src1, 5);
    printBuffer("Destination after copy:  ", dest1, 15);
    printf("\n");

    // Test Case 2: Partial copy
    printf("--- Test Case 2: Partial Copy ---\n");
    const char* src2 = "Programming";
    char dest2[16];
    memset(dest2, 0, 16);
    printBuffer("Destination before copy: ", dest2, 15);
    memoryCopy(dest2, src2, 4);
    printBuffer("Destination after copy:  ", dest2, 15);
    printf("\n");

    // Test Case 3: Overwriting existing data
    printf("--- Test Case 3: Overwriting Existing Data ---\n");
    const char* src3 = "NewData";
    char dest3[] = "OldDataOldData";
    printBuffer("Destination before copy: ", dest3, 14);
    memoryCopy(dest3, src3, 7);
    printBuffer("Destination after copy:  ", dest3, 14);
    printf("\n");

    // Test Case 4: Copying zero bytes
    printf("--- Test Case 4: Copying Zero Bytes ---\n");
    const char* src4 = "Source";
    char dest4[] = "Destination";
    char original_dest4[12];
    memcpy(original_dest4, dest4, 12);
    printBuffer("Destination before copy: ", dest4, 11);
    memoryCopy(dest4, src4, 0);
    printBuffer("Destination after copy:  ", dest4, 11);
    printf("Unchanged: %s\n", (memcmp(original_dest4, dest4, 12) == 0 ? "true" : "false"));
    printf("\n");

    // Test Case 5: Copying into a larger buffer
    printf("--- Test Case 5: Copying Into a Larger Buffer ---\n");
    const char* src5 = "Short";
    char dest5[11];
    memset(dest5, 'X', 10);
    dest5[10] = '\0';
    printBuffer("Destination before copy: ", dest5, 10);
    memoryCopy(dest5, src5, 5);
    printBuffer("Destination after copy:  ", dest5, 10);
    printf("\n");

    return 0;
}