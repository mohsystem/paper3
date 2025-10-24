#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 64

/**
 * @brief Safely copies a user-provided string into a fixed-size buffer.
 *
 * This function uses snprintf to prevent buffer overflows. It ensures that
 * the destination buffer is always null-terminated. If the source string
 * is larger than the buffer, it will be truncated.
 *
 * @param buffer The destination buffer.
 * @param buffer_size The total size of the destination buffer.
 * @param user_input The null-terminated string to copy.
 */
void handle_input(char* buffer, size_t buffer_size, const char* user_input) {
    if (buffer == NULL || buffer_size == 0) {
        return; // Do nothing if buffer is invalid
    }
    
    if (user_input == NULL) {
        buffer[0] = '\0'; // Handle null input by making buffer an empty string
        return;
    }

    // snprintf is a safe function for copying strings.
    // It copies at most buffer_size - 1 characters and always appends a null terminator.
    snprintf(buffer, buffer_size, "%s", user_input);
}

int main(void) {
    const char* test_cases[] = {
        "This is a short input.",
        "This is a very long input string that is definitely going to be longer than the sixty-four character buffer we have defined, thus causing truncation.",
        "",
        "123456789012345678901234567890123456789012345678901234567890123", // 63 chars
        "1234567890123456789012345678901234567890123456789012345678901234"  // 64 chars
    };
    
    for (int i = 0; i < 5; ++i) {
        char buffer[BUFFER_SIZE];
        
        // It's good practice to initialize the buffer, though handle_input is safe
        buffer[0] = '\0';

        printf("Test Case #%d\n", i + 1);
        printf("Input: \"%s\"\n", test_cases[i]);

        handle_input(buffer, BUFFER_SIZE, test_cases[i]);

        printf("Buffer Content: \"%s\"\n", buffer);
        printf("--------------------\n");
    }

    return 0;
}