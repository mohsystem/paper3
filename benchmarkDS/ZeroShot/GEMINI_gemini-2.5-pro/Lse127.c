#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define a reasonable maximum length for a username to prevent DoS attacks.
#define MAX_USERNAME_LEN 256
// Define buffer size for encoded output. Worst case is replacing 1 char with 6 (e.g., '"' -> "&quot;").
#define MAX_ENCODED_LEN (MAX_USERNAME_LEN * 6 + 1)

/**
 * Performs HTML entity encoding to prevent Cross-Site Scripting (XSS).
 *
 * @param input The raw null-terminated string to encode.
 * @param output The buffer to store the encoded string.
 * @param output_size The size of the output buffer.
 * @return 0 on success, -1 on output buffer overflow.
 */
int encodeHTML(const char *input, char *output, size_t output_size) {
    size_t out_idx = 0;
    if (output_size == 0) return -1; // No space at all

    for (size_t i = 0; input[i] != '\0'; i++) {
        const char *entity = NULL;
        size_t entity_len = 0;

        switch (input[i]) {
            case '&': entity = "&amp;";   entity_len = 5; break;
            case '<': entity = "&lt;";    entity_len = 4; break;
            case '>': entity = "&gt;";    entity_len = 4; break;
            case '"': entity = "&quot;";  entity_len = 6; break;
            case '\'': entity = "&#x27;"; entity_len = 5; break;
            case '/': entity = "&#x2F;"; entity_len = 5; break;
        }

        if (entity) {
            if (out_idx + entity_len >= output_size) return -1;
            memcpy(output + out_idx, entity, entity_len);
            out_idx += entity_len;
        } else {
            if (out_idx + 1 >= output_size) return -1;
            output[out_idx++] = input[i];
        }
    }
    
    // Null-terminate the output string
    output[out_idx] = '\0';
    return 0;
}

/**
 * Reads a username from a request stream, validates it, and writes a secure
 * "Hello [username]" greeting to a response stream.
 *
 * @param request A FILE stream for the client's request.
 * @param response A FILE stream for the server's response.
 */
void http_hello_page(FILE *request, FILE *response) {
    // Buffer for raw input. +2 for potential newline and null terminator.
    char username_raw[MAX_USERNAME_LEN + 2];
    
    // 1. Security: Use fgets for safe, bounded reading to prevent buffer overflows.
    if (fgets(username_raw, sizeof(username_raw), request) == NULL) {
        // Handle read error or EOF
        fprintf(response, "Hello stranger! (Error reading name)");
        return;
    }

    // 2. Security: Check for input that was too long.
    // If the buffer is full but doesn't contain a newline, the input was truncated.
    size_t len = strlen(username_raw);
    if (len > 0 && username_raw[len - 1] != '\n') {
        // Also check if we're at the end of the buffer
        if (len == sizeof(username_raw) - 1) {
            fprintf(response, "Hello! (Username is too long)");
            // In a real server, you should consume the rest of the oversized line
            // from the input stream to prevent it from affecting the next request.
            return;
        }
    }
    
    // Remove trailing newline character if it exists
    username_raw[strcspn(username_raw, "\r\n")] = '\0';

    // 3. Security: Sanitize output by HTML-encoding the username.
    char username_encoded[MAX_ENCODED_LEN];
    if (encodeHTML(username_raw, username_encoded, sizeof(username_encoded)) != 0) {
        fprintf(response, "Hello! (Error processing username)");
        return;
    }
    
    // Construct and write the response
    fprintf(response, "Hello %s", username_encoded);
}

// NOTE: fmemopen is a POSIX.1-2008 feature. Not available on Windows.
// This test harness will work on Linux and macOS.
#if defined(__linux__) || defined(__APPLE__)
#define HAS_FMEMOPEN 1
#endif

void run_test_case(int caseNum, const char* input) {
    printf("--- Test Case %d ---\n", caseNum);
    printf("Input: %s\n", input);

#ifdef HAS_FMEMOPEN
    // Create a read buffer for the input string
    char input_buffer[1024];
    strncpy(input_buffer, input, sizeof(input_buffer) - 2);
    strcat(input_buffer, "\n"); // Add newline as fgets expects it

    // Use a fixed-size buffer for the output
    char output_buffer[1024] = {0};

    // Simulate file streams in memory
    FILE* request_stream = fmemopen(input_buffer, strlen(input_buffer), "r");
    FILE* response_stream = fmemopen(output_buffer, sizeof(output_buffer), "w");

    if (request_stream && response_stream) {
        http_hello_page(request_stream, response_stream);
        fclose(request_stream);
        fclose(response_stream); // This flushes the stream to the buffer
        printf("Output: %s\n", output_buffer);
    } else {
        printf("Error: Could not open memory streams for testing.\n");
    }
#else
    printf("Output: (Skipping test: fmemopen not available on this system)\n");
#endif
    printf("\n");
}

int main() {
    // Generate a long string for testing
    char long_input[301];
    for (int i = 0; i < 300; ++i) long_input[i] = 'A';
    long_input[300] = '\0';

    // Test cases
    const char* test_inputs[] = {
        // Test Case 1: Normal valid username
        "Alice",
        // Test Case 2: XSS attack attempt
        "<script>alert('xss')</script>",
        // Test Case 3: Input length violation
        long_input,
        // Test Case 4: Empty username
        "",
        // Test Case 5: Username with special characters that need encoding
        "Bob&Eve"
    };
    
    int num_tests = sizeof(test_inputs) / sizeof(test_inputs[0]);
    for (int i = 0; i < num_tests; i++) {
        run_test_case(i + 1, test_inputs[i]);
    }

    return 0;
}