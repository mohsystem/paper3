#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * NOTE: Standard C has no built-in libraries for creating a web application.
 * This would require external libraries like libmicrohttpd or Mongoose.
 * The following code simulates the core logic of safely parsing a payload
 * string, as a full web server cannot be implemented in a single standard C file.
 */

/**
 * @brief Securely finds and extracts the value of the "name" field from a payload.
 *
 * This function searches for a "name":"value" pattern in the payload string and copies
 * the value into a provided buffer, ensuring no buffer overflows occur.
 *
 * @param payload The input string, e.g., from a request body.
 * @param out_buffer The buffer to store the extracted name.
 * @param buffer_size The total size of out_buffer.
 * @return 0 on success, -1 if the field is not found or malformed, -2 if the buffer is too small.
 */
int checkNameInPayload(const char* payload, char* out_buffer, size_t buffer_size) {
    if (payload == NULL || out_buffer == NULL || buffer_size == 0) {
        return -1; // Invalid arguments
    }
    
    // Clear the output buffer to ensure it's clean
    memset(out_buffer, 0, buffer_size);

    const char* key = "\"name\":\"";
    const char* start_ptr = strstr(payload, key);

    if (start_ptr == NULL) {
        return -1; // Key not found
    }

    start_ptr += strlen(key); // Move pointer to the start of the value

    const char* end_ptr = strchr(start_ptr, '"');

    if (end_ptr == NULL) {
        return -1; // Malformed, no closing quote
    }

    size_t value_len = end_ptr - start_ptr;

    // Security: Check if the extracted value will fit in the buffer, including null terminator.
    if (value_len >= buffer_size) {
        return -2; // Buffer too small
    }

    // Security: Use strncpy to safely copy the value, then null-terminate.
    strncpy(out_buffer, start_ptr, value_len);
    out_buffer[value_len] = '\0'; // Ensure null termination

    return 0; // Success
}

int main() {
    printf("--- C Payload Processing Simulation ---\n\n");

    const char* test_cases[] = {
        // Test Case 1: Valid payload
        "{\"id\":123, \"name\":\"Frank\"}",
        // Test Case 2: No 'name' field
        "{\"id\":456, \"status\":\"active\"}",
        // Test Case 3: Empty payload
        "",
        // Test Case 4: Malformed payload
        "{\"name\":\"Grace",
        // Test Case 5: Payload with a long name to test buffer limits
        "{\"name\":\"ThisIsAVeryLongNameThatIsDesignedToExceedTheBufferSizeOfTheSmallTestBufferAndTriggerTheSecurityCheck\"}"
    };
    
    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    char name_buffer[64];

    for (size_t i = 0; i < num_tests; ++i) {
        printf("Test Case %zu:\n", i + 1);
        printf("  Payload: %s\n", test_cases[i]);

        int result = checkNameInPayload(test_cases[i], name_buffer, sizeof(name_buffer));
        
        printf("  Result: ");
        if (result == 0) {
            // In a web context, this output must be properly encoded before rendering.
            printf("Success! Name = \"%s\"\n", name_buffer);
        } else if (result == -1) {
            printf("Error: Field 'name' not found or payload is malformed.\n");
        } else if (result == -2) {
            printf("Error: Buffer too small for the extracted name.\n");
        }
        printf("---\n");
    }

    return 0;
}