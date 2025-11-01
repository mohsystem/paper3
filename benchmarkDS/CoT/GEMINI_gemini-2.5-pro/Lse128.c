#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERNAME_LEN 256
// Worst-case expansion: '&' becomes '&amp;' (5 chars). We use 6 for safety.
#define MAX_SANITIZED_LEN (MAX_USERNAME_LEN * 6)
#define MAX_RESPONSE_LEN (MAX_SANITIZED_LEN + 10) // "hello " + sanitized + null terminator

/**
 * @brief Sanitizes a source string into a destination buffer for HTML output.
 * 
 * Replaces special HTML characters with their entities. This function is
 * designed to be safe against buffer overflows by checking the destination
 * buffer size at each step.
 *
 * @param src The source string to sanitize.
 * @param dest The destination buffer to write the sanitized string to.
 * @param dest_size The total size of the destination buffer.
 */
void escapeHtml(const char* src, char* dest, size_t dest_size) {
    if (!src || !dest || dest_size == 0) {
        if (dest && dest_size > 0) dest[0] = '\0';
        return;
    }

    size_t dest_idx = 0;
    const char* entity = NULL;
    size_t entity_len = 0;

    for (size_t i = 0; src[i] != '\0'; ++i) {
        switch (src[i]) {
            case '<': entity = "&lt;";   entity_len = 4; break;
            case '>': entity = "&gt;";   entity_len = 4; break;
            case '&': entity = "&amp;";  entity_len = 5; break;
            case '"': entity = "&quot;"; entity_len = 6; break;
            case '\'': entity = "&#39;"; entity_len = 5; break;
            default:  entity = NULL;    entity_len = 0; break;
        }

        if (entity) {
            // Check if there's enough space for the entity and the null terminator
            if (dest_idx + entity_len < dest_size) {
                memcpy(dest + dest_idx, entity, entity_len);
                dest_idx += entity_len;
            } else {
                // Not enough space, truncate and terminate
                break;
            }
        } else {
            // Check if there's enough space for the character and the null terminator
            if (dest_idx + 1 < dest_size) {
                dest[dest_idx++] = src[i];
            } else {
                // Not enough space, truncate and terminate
                break;
            }
        }
    }
    dest[dest_idx] = '\0'; // Null-terminate the destination string
}


/**
 * @brief Reads a username from a request stream, sanitizes it, and writes a
 * "hello [username]" message to the response stream.
 *
 * @param request A readable file stream (e.g., stdin, file).
 * @param response A writable file stream (e.g., stdout, file).
 */
void http_hello_page(FILE* request, FILE* response) {
    char username[MAX_USERNAME_LEN];
    char sanitized_username[MAX_SANITIZED_LEN];
    char response_body[MAX_RESPONSE_LEN];

    // Safely read one line from the request stream, preventing buffer overflow.
    if (fgets(username, sizeof(username), request) != NULL) {
        // fgets may include a newline character, so we remove it.
        username[strcspn(username, "\r\n")] = '\0';
    } else {
        // Handle empty or failed read by setting username to an empty string.
        username[0] = '\0';
    }

    // Sanitize the username to prevent XSS.
    escapeHtml(username, sanitized_username, sizeof(sanitized_username));

    // Safely construct the final response string, preventing buffer overflow.
    snprintf(response_body, sizeof(response_body), "hello %s", sanitized_username);

    // Write the response to the output stream.
    fputs(response_body, response);
}

void run_test(int test_num, const char* username, const char* expected) {
    // We use temporary files to simulate streams, as this is the most portable C approach.
    FILE *request_file = tmpfile();
    FILE *response_file = tmpfile();
    
    if (!request_file || !response_file) {
        perror("Failed to create temporary files");
        return;
    }

    // Write test input to the request file
    fputs(username, request_file);
    fputs("\n", request_file);
    rewind(request_file); // Go back to the beginning of the file

    // Call the function
    http_hello_page(request_file, response_file);

    // Read the result from the response file
    rewind(response_file);
    char actual[MAX_RESPONSE_LEN] = {0};
    if (fgets(actual, sizeof(actual), response_file) == NULL && ferror(response_file)) {
        perror("Error reading from response file");
    }
    
    // Clean up files (they are deleted on close)
    fclose(request_file);
    fclose(response_file);

    // Compare and print results
    printf("Test Case %d:\n", test_num);
    printf("  Input:    \"%s\"\n", username);
    printf("  Expected: \"%s\"\n", expected);
    printf("  Actual:   \"%s\"\n", actual);
    printf("  Result:   %s\n\n", strcmp(expected, actual) == 0 ? "PASSED" : "FAILED");
}


int main() {
    printf("--- Running C Tests ---\n");

    run_test(1, "Alice", "hello Alice");
    run_test(2, "Bob Smith", "hello Bob Smith");
    run_test(3, "", "hello ");
    run_test(4, "<script>alert('XSS')</script>", "hello &lt;script&gt;alert(&#39;XSS&#39;)&lt;/script&gt;");
    run_test(5, "User with \"&'<> special chars", "hello User with &quot;&amp;&#39;&lt;&gt; special chars");

    return 0;
}