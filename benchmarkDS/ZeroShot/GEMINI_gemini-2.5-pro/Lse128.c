#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Securely escapes a string for an HTML context to prevent XSS.
 * 
 * @param input The null-terminated string to escape.
 * @param output The buffer to write the escaped string to.
 * @param out_size The total size of the output buffer.
 * @return 0 on success, -1 on failure (e.g., output buffer too small).
 */
int html_escape(const char *input, char *output, size_t out_size) {
    size_t out_idx = 0;
    if (!input || !output || out_size == 0) return -1;

    for (size_t i = 0; input[i] != '\0'; ++i) {
        const char *replacement = NULL;
        switch (input[i]) {
            case '&':  replacement = "&amp;";  break;
            case '\"': replacement = "&quot;"; break;
            case '\'': replacement = "&#39;";  break;
            case '<':  replacement = "&lt;";   break;
            case '>':  replacement = "&gt;";   break;
        }

        if (replacement) {
            size_t len = strlen(replacement);
            if (out_idx + len >= out_size) {
                output[out_idx] = '\0'; // Ensure null-termination on error
                return -1; 
            }
            memcpy(output + out_idx, replacement, len);
            out_idx += len;
        } else {
            if (out_idx + 1 >= out_size) {
                output[out_idx] = '\0'; // Ensure null-termination on error
                return -1;
            }
            output[out_idx++] = input[i];
        }
    }
    output[out_idx] = '\0';
    return 0;
}

/**
 * @brief Reads a username from a request stream, constructs a secure "hello [username]"
 * HTML response, and writes it to the response stream.
 *
 * @param request A FILE stream for the request, assumed to contain the username.
 * @param response A FILE stream for the response.
 */
void http_hello_page(FILE *request, FILE *response) {
    char username[256];
    // Output buffer needs to be larger to hold escaped characters.
    // Worst case: '&' -> "&amp;" (5 chars) or '\'' -> "&#39;" (5 chars).
    // A 6x buffer is a safe general estimate.
    char escaped_username[sizeof(username) * 6];

    if (fgets(username, sizeof(username), request) == NULL) {
        strcpy(username, ""); // Handle error or EOF by using an empty username
    }

    // Remove trailing newline character from fgets if it exists
    username[strcspn(username, "\r\n")] = 0;

    if (html_escape(username, escaped_username, sizeof(escaped_username)) != 0) {
        fprintf(stderr, "Error: Escaped username buffer is too small.\n");
        fprintf(response, "HTTP/1.1 500 Internal Server Error\r\n\r\nError escaping username.");
        fflush(response);
        return;
    }

    fprintf(response, "HTTP/1.1 200 OK\r\n"
                      "Content-Type: text/html; charset=UTF-8\r\n\r\n"
                      "<html><body>hello %s</body></html>", escaped_username);
    fflush(response);
}

int main(void) {
    const char *test_inputs[] = {
        "Alice",                                  // Test case 1: Normal input
        "Bob Smith",                              // Test case 2: Input with spaces
        "",                                       // Test case 3: Empty input
        "<script>alert('XSS')</script>",          // Test case 4: XSS attack attempt
        "John&Doe\" onload=\"alert('hacked')\""   // Test case 5: XSS attack with quotes and ampersand
    };
    int num_tests = sizeof(test_inputs) / sizeof(test_inputs[0]);

    for (int i = 0; i < num_tests; ++i) {
        // Use temporary files to simulate request and response streams
        FILE *req_stream = tmpfile();
        FILE *res_stream = tmpfile();

        if (!req_stream || !res_stream) {
            perror("Failed to create temporary files");
            return 1;
        }

        printf("--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", test_inputs[i]);

        fputs(test_inputs[i], req_stream);
        rewind(req_stream);

        http_hello_page(req_stream, res_stream);
        
        rewind(res_stream);
        printf("Output:\n");
        char buffer[1024];
        while (fgets(buffer, sizeof(buffer), res_stream) != NULL) {
            printf("%s", buffer);
        }
        printf("\n\n");

        fclose(req_stream);
        fclose(res_stream);
    }

    return 0;
}