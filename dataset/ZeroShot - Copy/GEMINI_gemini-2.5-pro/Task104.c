#include <stdio.h>
#include <string.h>

/**
 * @brief Securely copies a source string into a fixed-size destination buffer.
 *
 * This function uses strncpy to prevent buffer overflows by copying at most
 * dest_size - 1 characters from the source string. It then explicitly
 * null-terminates the destination buffer to ensure it is always a valid C-string.
 *
 * @param dest Pointer to the destination character buffer.
 * @param dest_size The total size of the destination buffer (including space for null terminator).
 * @param src Pointer to the null-terminated source string.
 */
void handleInput(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || dest_size == 0) {
        return; // Nothing to do
    }
    
    // Copy at most dest_size - 1 characters. strncpy is safer than strcpy.
    strncpy(dest, src, dest_size - 1);

    // Explicitly null-terminate the buffer. strncpy does not guarantee
    // null-termination if the source string is as long as or longer than dest_size.
    dest[dest_size - 1] = '\0';
}

int main() {
    const int BUFFER_SIZE = 16;

    // Test Cases
    const char* testInputs[] = {
        "Short string",      // Case 1: Shorter than buffer
        "This is too long",  // Case 2: Longer than buffer
        "Exactly 15 char",   // Case 3: Exactly fits (15 chars + null)
        "",                  // Case 4: Empty string
        "Sixteen chars!!",   // Case 5: Source is exactly 16 chars, will be truncated
    };
    int num_tests = sizeof(testInputs) / sizeof(testInputs[0]);

    printf("C Test Cases (Buffer Size: %d)\n", BUFFER_SIZE);
    printf("------------------------------------\n");

    for (int i = 0; i < num_tests; ++i) {
        const char* original = testInputs[i];
        char buffer[BUFFER_SIZE];
        
        handleInput(buffer, BUFFER_SIZE, original);

        printf("Test Case %d:\n", i + 1);
        printf("  Original:  \"%s\" (length: %zu)\n", original, strlen(original));
        printf("  Processed: \"%s\" (length: %zu)\n\n", buffer, strlen(buffer));
    }

    return 0;
}