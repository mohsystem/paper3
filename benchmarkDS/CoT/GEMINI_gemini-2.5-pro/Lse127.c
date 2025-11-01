#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Sanitizes a string to prevent XSS by escaping HTML special characters.
 * 
 * This function allocates a new string for the sanitized output. The caller is
 * responsible for freeing this memory using free().
 *
 * @param input The raw input string.
 * @return A newly allocated, sanitized string, or NULL on failure.
 */
char* sanitize_html(const char* input) {
    if (!input) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    size_t new_len = 0;
    // First pass: calculate the required length for the new string
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '<': case '>': new_len += 4; break; // &lt; &gt;
            case '&': new_len += 5; break;           // &amp;
            case '"': new_len += 6; break;           // &quot;
            case '\'': new_len += 5; break;          // &#39;
            default: new_len++; break;
        }
    }

    char* sanitized = (char*)malloc(new_len + 1);
    if (!sanitized) {
        perror("Failed to allocate memory for sanitized string");
        return NULL;
    }

    size_t j = 0;
    // Second pass: build the new sanitized string
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '<': strcpy(sanitized + j, "&lt;"); j += 4; break;
            case '>': strcpy(sanitized + j, "&gt;"); j += 4; break;
            case '&': strcpy(sanitized + j, "&amp;"); j += 5; break;
            case '"': strcpy(sanitized + j, "&quot;"); j += 6; break;
            case '\'': strcpy(sanitized + j, "&#39;"); j += 5; break;
            default: sanitized[j++] = input[i]; break;
        }
    }
    sanitized[j] = '\0';
    return sanitized;
}

/**
 * @brief Reads a username from a request stream, constructs a "Hello [username]" message,
 *        and writes it to a response stream. The username is sanitized to prevent XSS.
 *
 * @param request A FILE stream to read the username from.
 * @param response A FILE stream to write the response to.
 */
void http_hello_page(FILE* request, FILE* response) {
    // Use a fixed-size buffer to prevent buffer overflows.
    char username[256];

    // Read a line safely using fgets.
    if (fgets(username, sizeof(username), request) == NULL) {
        // Handle empty or failed read, assume default.
        strcpy(username, ""); 
    }

    // Remove trailing newline character, if present.
    size_t len = strlen(username);
    if (len > 0 && username[len - 1] == '\n') {
        username[len - 1] = '\0';
    }

    // Sanitize the username to prevent XSS.
    char* sanitized_username = sanitize_html(username);
    if (!sanitized_username) {
        // Handle memory allocation failure gracefully.
        fprintf(response, "Hello Guest");
        return;
    }

    // Construct and write the response.
    fprintf(response, "Hello %s", sanitized_username);
    
    // Free the memory allocated by sanitize_html.
    free(sanitized_username);
}

void run_test_case(int test_num, const char* test_input, const char* expected_output) {
    printf("Test Case %d:\n", test_num);
    printf("Input: \"%s\"\n", test_input);
    
    // Use temporary files to simulate file streams
    FILE *request_stream = tmpfile();
    FILE *response_stream = tmpfile();

    if (!request_stream || !response_stream) {
        perror("Failed to create temporary files");
        if (request_stream) fclose(request_stream);
        if (response_stream) fclose(response_stream);
        printf("Result  : FAIL (Setup error)\n--------------------\n");
        return;
    }

    // Write input to the request stream
    fputs(test_input, request_stream);
    fputc('\n', request_stream);
    rewind(request_stream);

    // Run the function
    http_hello_page(request_stream, response_stream);

    // Read the output from the response stream
    rewind(response_stream);
    char actual_output[512] = {0};
    if (fgets(actual_output, sizeof(actual_output), response_stream) == NULL) {
        // If nothing was written, actual_output remains an empty string.
    }
    
    fclose(request_stream);
    fclose(response_stream);

    printf("Expected: \"%s\"\n", expected_output);
    printf("Actual  : \"%s\"\n", actual_output);
    printf("Result  : %s\n", strcmp(expected_output, actual_output) == 0 ? "PASS" : "FAIL");
    printf("--------------------\n");
}


int main() {
    // Test cases
    run_test_case(1, "Alice", "Hello Alice");
    run_test_case(2, "<script>alert('xss')</script>", "Hello &lt;script&gt;alert('xss')&lt;/script&gt;");
    run_test_case(3, "Bob & Carol", "Hello Bob &amp; Carol");
    run_test_case(4, "Mr. \"O'Malley\"", "Hello Mr. &quot;O&#39;Malley&quot;");
    run_test_case(5, "", "Hello ");

    return 0;
}