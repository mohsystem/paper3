#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERNAME_LEN 128
// Worst case: every char is '"' -> &quot; (6 chars)
#define MAX_ESCAPED_USERNAME_LEN (MAX_USERNAME_LEN * 6 + 1)
#define MAX_OUTPUT_BUF_LEN 1024

/**
 * @brief Sanitizes a string for HTML context to prevent XSS.
 * @param dest The destination buffer for the escaped string.
 * @param dest_size The size of the destination buffer.
 * @param src The source string to escape.
 * @return 0 on success, -1 on failure (e.g., buffer too small).
 */
int html_escape(char *dest, size_t dest_size, const char *src) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }
    
    char *d_ptr = dest;
    const char *s_ptr = src;
    size_t remaining = dest_size;
    
    // Clear destination buffer for safety
    memset(dest, 0, dest_size);

    while (*s_ptr != '\0') {
        const char *repl = NULL;
        size_t repl_len = 0;

        switch (*s_ptr) {
            case '&': repl = "&amp;"; repl_len = 5; break;
            case '<': repl = "&lt;"; repl_len = 4; break;
            case '>': repl = "&gt;"; repl_len = 4; break;
            case '\"': repl = "&quot;"; repl_len = 6; break;
            case '\'': repl = "&#39;"; repl_len = 5; break;
            default: break;
        }

        if (repl) {
            if (remaining <= repl_len) { // Check includes space for null terminator
                return -1; // Not enough space
            }
            memcpy(d_ptr, repl, repl_len);
            d_ptr += repl_len;
            remaining -= repl_len;
        } else {
            if (remaining <= 1) {
                return -1; // Not enough space
            }
            *d_ptr = *s_ptr;
            d_ptr++;
            remaining--;
        }
        s_ptr++;
    }
    *d_ptr = '\0'; // Null-terminate
    return 0;
}


/**
 * @brief Simulates handling a /hello request.
 * Reads username from request_body, constructs and writes response to outputbuf.
 * In a real server, these would be streams, not simple char buffers.
 * This implementation uses buffers for a self-contained, portable example.
 *
 * @param request_stream Input stream containing the username.
 * @param response_stream Output stream for the HTML response.
 */
void http_hello_page(FILE *request_stream, FILE *response_stream) {
    char username[MAX_USERNAME_LEN + 1];
    char escaped_username[MAX_ESCAPED_USERNAME_LEN];
    char outputbuf[MAX_OUTPUT_BUF_LEN];

    // The server reads the request body into the username variable.
    // Use fgets for safe, bounded reading from a stream.
    if (fgets(username, sizeof(username), request_stream) == NULL) {
        strcpy(username, "Guest"); // Default if read fails or is empty
    }
    
    // Remove trailing newline characters if present
    username[strcspn(username, "\r\n")] = '\0';
    if(strlen(username) == 0) {
        strcpy(username, "Guest");
    }

    // Sanitize the username for use in HTML
    if (html_escape(escaped_username, sizeof(escaped_username), username) != 0) {
        fprintf(response_stream, "<html><body><h1>Error: Username is invalid or too long to process.</h1></body></html>");
        return;
    }
    
    // The server constructs the response body into the outputbuf variable.
    const char *html_template = "<html><body><h1>Hello, %s!</h1></body></html>";
    int written = snprintf(outputbuf, sizeof(outputbuf), html_template, escaped_username);

    // The server writes the response body to the response stream.
    if (written < 0 || (size_t)written >= sizeof(outputbuf)) {
        // This case indicates an error or truncation in snprintf
        fprintf(response_stream, "<html><body><h1>Internal Server Error.</h1></body></html>");
    } else {
        fputs(outputbuf, response_stream);
    }

    // The server closes the response stream. This is typically handled by the server
    // framework that opened the stream. The function itself should not close
    // streams it did not open.
}

int main() {
    const char *test_cases[] = {
        "Alice",
        "Bob<script>alert(1)</script>",
        "", // Empty username
        "Charles & David",
        "Eve's \"special\" username that is quite long to test the boundaries and see how it behaves with more characters"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_test_cases; ++i) {
        printf("--- Test Case %d ---\n", i + 1);
        printf("Input Username: %s\n", test_cases[i]);
        
        // Simulate streams using temporary files for portability
        FILE* req_stream = tmpfile();
        FILE* res_stream = tmpfile();

        if (req_stream == NULL || res_stream == NULL) {
            perror("Failed to create temporary files for streams");
            return 1;
        }

        // Write test case to the request "stream"
        fputs(test_cases[i], req_stream);
        rewind(req_stream);

        // Call the function
        http_hello_page(req_stream, res_stream);

        // Read the result from the response "stream"
        rewind(res_stream);
        char result_buffer[MAX_OUTPUT_BUF_LEN] = {0};
        if (fread(result_buffer, 1, sizeof(result_buffer) - 1, res_stream) == 0 && ferror(res_stream)){
            perror("Failed to read from response stream");
        }
        
        printf("Output HTML: %s\n\n", result_buffer);

        // Close streams
        fclose(req_stream);
        fclose(res_stream);
    }
    
    return 0;
}