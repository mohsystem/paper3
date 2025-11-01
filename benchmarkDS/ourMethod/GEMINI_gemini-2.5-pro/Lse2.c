#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Copies a number of bytes from one memory location to another.
 *
 * @warning This function is inherently unsafe as it cannot verify the size of the
 * destination buffer. The caller is solely responsible for ensuring that the
 * destination buffer `dest` is large enough to hold `n` bytes to prevent
 * buffer overflows. Secure usage requires the caller to perform bounds checks
 * before calling this function.
 *
 * @param dest Pointer to the destination memory location. Must not be NULL.
 * @param src Pointer to the source memory location. Must not be NULL.
 * @param n The number of bytes to copy.
 */
void copyBytes(void* dest, const void* src, size_t n) {
    if (dest == NULL || src == NULL) {
        // In a real application, this should be logged or handled.
        return;
    }
    if (n == 0) {
        return;
    }

    // Note: memcpy has undefined behavior if src and dest overlap.
    // Use memmove for overlapping regions. Assuming non-overlapping here.
    memcpy(dest, src, n);
}

void print_buffer(const char* name, const char* buffer, size_t len) {
    printf("%s: \"", name);
    // Use fwrite to safely print a specific number of bytes,
    // which may not be null-terminated.
    fwrite(buffer, 1, len, stdout);
    printf("\"\n");
}

void test_case_1(void) {
    printf("--- Test Case 1: Normal Copy ---\n");
    const char* src = "Hello, C!";
    const size_t src_len_with_null = strlen(src) + 1;
    const size_t dest_size = 50;
    char* dest = (char*)malloc(dest_size);
    if (dest == NULL) {
        perror("malloc failed");
        return;
    }
    memset(dest, 0, dest_size);

    // Rule #1, #2: Caller performs the boundary check
    if (src_len_with_null <= dest_size) {
        printf("Attempting to copy %zu bytes.\n", src_len_with_null);
        copyBytes(dest, src, src_len_with_null);
        printf("Copy successful.\n");
        print_buffer("Destination", dest, strlen(src));
    } else {
        printf("Copy failed: Destination buffer too small.\n");
    }
    free(dest);
    printf("\n");
}

void test_case_2(void) {
    printf("--- Test Case 2: Destination Too Small (Checked) ---\n");
    const char* src = "This message is too long";
    const size_t src_len_with_null = strlen(src) + 1;
    const size_t dest_size = 10;
    char dest[dest_size];
    
    printf("Attempting to copy %zu bytes into a %zu-byte buffer.\n", src_len_with_null, dest_size);
    // Rule #1, #2: The crucial safety check before calling the unsafe function
    if (src_len_with_null <= dest_size) {
        copyBytes(dest, src, src_len_with_null);
        printf("Copy successful.\n");
    } else {
        printf("Copy aborted by caller's check, as expected.\n");
    }
    printf("\n");
}

void test_case_3(void) {
    printf("--- Test Case 3: Zero Bytes Copy ---\n");
    const char* src = "some data";
    const size_t dest_size = 20;
    char dest[dest_size];
    memset(dest, 'A', dest_size - 1);
    dest[dest_size-1] = '\0';
    char original_dest[dest_size];
    memcpy(original_dest, dest, dest_size);

    size_t n = 0;
    printf("Attempting to copy %zu bytes.\n", n);
    copyBytes(dest, src, n);
    printf("Function called.\n");
    if (memcmp(dest, original_dest, dest_size) == 0) {
        printf("Destination buffer remains unchanged as expected.\n");
    } else {
        printf("Error: Destination buffer was modified.\n");
    }
    printf("\n");
}

void test_case_4(void) {
    printf("--- Test Case 4: Exact Size Copy ---\n");
    const char* src = "exact fit";
    const size_t src_len_with_null = strlen(src) + 1;
    const size_t dest_size = src_len_with_null;
    char* dest = (char*)malloc(dest_size);
    if (dest == NULL) {
        perror("malloc failed");
        return;
    }

    if (src_len_with_null <= dest_size) {
        printf("Attempting to copy %zu bytes into a %zu-byte buffer.\n", src_len_with_null, dest_size);
        copyBytes(dest, src, src_len_with_null);
        printf("Copy successful.\n");
        print_buffer("Destination", dest, strlen(src));
    } else {
        printf("Copy failed.\n");
    }
    free(dest);
    printf("\n");
}

void test_case_5(void) {
    printf("--- Test Case 5: Partial Copy ---\n");
    const char* src = "A complete message";
    const size_t n = 8; // copy "complete"
    const size_t dest_size = 50;
    char dest[dest_size];
    memset(dest, 0, dest_size);

    if (strlen(src) >= 2 + n && n < dest_size) {
        printf("Attempting to copy %zu bytes from an offset.\n", n);
        copyBytes(dest, src + 2, n);
        dest[n] = '\0'; // Manually null-terminate
        printf("Copy successful.\n");
        print_buffer("Destination", dest, n);
    } else {
        printf("Copy failed due to pre-condition check.\n");
    }
    printf("\n");
}

int main(void) {
    test_case_1();
    test_case_2();
    test_case_3();
    test_case_4();
    test_case_5();
    
    return 0;
}