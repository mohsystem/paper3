#include <stdio.h>
#include <string.h> // For memmove, memset
#include <stdlib.h> // For malloc, free
#include <ctype.h>  // For isprint

/**
 * @brief Copies a number of bytes from one memory location to another.
 *
 * This function is a wrapper around memmove. It is the caller's responsibility
 * to ensure that the destination buffer is large enough to hold 'num_bytes'
 * and that the source buffer contains at least 'num_bytes'.
 * This implementation adds checks for NULL pointers.
 * memmove is used instead of memcpy to correctly handle overlapping memory regions.
 *
 * @param dest Pointer to the destination memory block.
 * @param src Pointer to the source memory block.
 * @param num_bytes The number of bytes to copy.
 */
void copyBytes(void* dest, const void* src, size_t num_bytes) {
    // Rule #4: Validate inputs. Check for null pointers.
    if (dest == NULL || src == NULL) {
        // Silently return on null input to prevent crashes.
        // A more robust implementation might log an error.
        return;
    }

    if (num_bytes == 0) {
        return; // Nothing to copy.
    }

    // Rules #1, #2, #5: Use a safer function (memmove) for the copy.
    // memmove correctly handles overlapping memory regions, which is safer
    // than memcpy. The core responsibility for bounds checking for raw
    // pointers lies with the caller. This function cannot know the allocated
    // sizes of `dest` and `src`.
    memmove(dest, src, num_bytes);
}

void print_buffer(const char* name, const char* buffer, size_t size) {
    printf("%s: ", name);
    for (size_t i = 0; i < size; ++i) {
        printf("%02x ", (unsigned char)buffer[i]);
    }
    printf("(str: \"");
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] != '\0' && isprint((unsigned char)buffer[i])) {
            putchar(buffer[i]);
        } else {
            putchar('.');
        }
    }
    printf("\")\n");
}


int main() {
    // Test Case 1: Normal successful copy
    printf("--- Test Case 1: Normal Copy ---\n");
    char src1[] = "Hello"; // size 6 (includes '\0')
    char* dest1 = (char*)malloc(6 * sizeof(char));
    if (dest1 == NULL) {
        perror("malloc failed");
        return 1;
    }
    memset(dest1, 0, 6);
    copyBytes(dest1, src1, 6);
    print_buffer("Source     ", src1, 6);
    print_buffer("Destination", dest1, 6);
    free(dest1);
    printf("\n");

    // Test Case 2: Overlapping memory (dest starts within src)
    printf("--- Test Case 2: Overlapping Copy (dest > src) ---\n");
    char buffer2[] = "123456789"; // size 10
    print_buffer("Before", buffer2, 10);
    copyBytes(buffer2 + 2, buffer2, 5); // copy "12345" to buffer[2]
    print_buffer("After ", buffer2, 10);
    printf("\n");

    // Test Case 3: Overlapping memory (src starts within dest)
    printf("--- Test Case 3: Overlapping Copy (src > dest) ---\n");
    char buffer3[] = "123456789"; // size 10
    print_buffer("Before", buffer3, 10);
    copyBytes(buffer3, buffer3 + 2, 5); // copy "34567" to buffer[0]
    print_buffer("After ", buffer3, 10);
    printf("\n");
    
    // Test Case 4: Copy zero bytes
    printf("--- Test Case 4: Zero Bytes Copy ---\n");
    char src4[] = "ABC";
    char dest4[] = "XYZ";
    print_buffer("Dest Before", dest4, 4);
    copyBytes(dest4, src4, 0);
    print_buffer("Dest After ", dest4, 4);
    printf("Result: Destination is unchanged\n\n");

    // Test Case 5: Null pointer input
    printf("--- Test Case 5: Null Input ---\n");
    char src5[] = "ABC";
    copyBytes(NULL, src5, 3);
    copyBytes(src5, NULL, 3);
    printf("Result: Program did not crash (as expected).\n");
    print_buffer("Src (should be unchanged)", src5, 4);
    printf("\n");
    
    return 0;
}