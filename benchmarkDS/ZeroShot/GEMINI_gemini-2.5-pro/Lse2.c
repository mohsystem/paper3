#include <stdio.h>
#include <string.h> // For memmove and memset
#include <stddef.h> // For size_t
#include <ctype.h>  // For isprint

/**
 * @brief Securely copies a specified number of bytes from a source to a destination buffer.
 *        This function guards against null pointers and destination buffer overflows.
 * 
 * @param dest Pointer to the destination buffer.
 * @param dest_size The total allocated size of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param count The number of bytes to copy.
 * @return 0 on success, -1 on error.
 */
int secure_copy(void* dest, size_t dest_size, const void* src, size_t count) {
    // 1. Check for null pointers
    if (dest == NULL || src == NULL) {
        fprintf(stderr, "Error: Source or destination pointer is null.\n");
        return -1;
    }

    // 2. Check for destination buffer overflow
    if (count > dest_size) {
        fprintf(stderr, "Error: Copy would result in a buffer overflow on the destination.\n");
        return -1;
    }
    
    // 3. Trivial case: copy zero bytes, which is a success.
    if (count == 0) {
        return 0;
    }

    // Use memmove as it is safe for overlapping memory regions
    memmove(dest, src, count);

    return 0;
}

// Helper function to print buffers for testing
void print_buffer_c(const char* name, const char* buffer, size_t size) {
    printf("%s: \"", name);
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] == '\0') break; // Stop at first null for cleaner printing
        if (isprint((unsigned char)buffer[i])) {
            putchar(buffer[i]);
        } else {
            putchar('.');
        }
    }
    printf("\"\n");
}


int main() {
    // Test Case 1: Normal successful copy
    printf("--- Test Case 1: Normal Copy ---\n");
    const char* src1 = "Hello World";
    char dest1[20];
    memset(dest1, 0, 20);
    int result1 = secure_copy(dest1, 20, src1, 11);
    printf("Result: %s\n", (result1 == 0 ? "Success" : "Failure"));
    if(result1 == 0) print_buffer_c("Destination", dest1, 20);
    printf("\n");

    // Test Case 2: Boundary case (full buffer)
    printf("--- Test Case 2: Full Buffer Copy ---\n");
    const char* src2 = "Test";
    char dest2[4];
    int result2 = secure_copy(dest2, 4, src2, 4);
    printf("Result: %s\n", (result2 == 0 ? "Success" : "Failure"));
    if(result2 == 0) printf("Destination: \"%.*s\"\n", 4, dest2);
    printf("\n");

    // Test Case 3: Destination buffer overflow attempt
    printf("--- Test Case 3: Destination Overflow Attempt ---\n");
    const char* src3 = "This is way too long";
    char dest3[5];
    memset(dest3, 'X', 4);
    dest3[4] = '\0';
    int result3 = secure_copy(dest3, 5, src3, strlen(src3));
    printf("Result: %s\n", (result3 == 0 ? "Success" : "Failure"));
    print_buffer_c("Destination (should be unchanged)", dest3, 5);
    printf("\n");

    // Test Case 4: Source over-read attempt
    // Note: This function cannot prevent source over-reads as the source size is unknown.
    // The caller is responsible for ensuring 'count' <= size of 'src'.
    printf("--- Test Case 4: Valid copy (demonstrating caller responsibility) ---\n");
    const char* src4 = "short"; // length is 5
    char dest4[20];
    memset(dest4, 0, 20);
    printf("Note: C/C++ cannot prevent source over-reads without knowing source size.\n");
    int result4 = secure_copy(dest4, 20, src4, strlen(src4));
    printf("Result: %s\n", (result4 == 0 ? "Success" : "Failure"));
    if(result4 == 0) print_buffer_c("Destination", dest4, 20);
    printf("\n");

    // Test Case 5: Null pointer input
    printf("--- Test Case 5: Null Input ---\n");
    char dest5[10];
    const char* src5 = "test";
    int result5_a = secure_copy(NULL, 10, src5, 4);
    printf("Result (null destination): %s\n", (result5_a == 0 ? "Success" : "Failure"));
    int result5_b = secure_copy(dest5, 10, NULL, 4);
    printf("Result (null source): %s\n", (result5_b == 0 ? "Success" : "Failure"));

    return 0;
}