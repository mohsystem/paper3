#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 1024
#define MAX_USERNAME_LENGTH 50
#define MAX_REQUEST_LINES 10

/**
 * @brief Validates that a username candidate is safe.
 *
 * @param username_candidate The string to validate.
 * @return true if the username is valid, false otherwise.
 */
static bool is_username_valid(const char* username_candidate) {
    if (username_candidate == NULL) {
        return false;
    }
    size_t len = strlen(username_candidate);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username_candidate[i]) && username_candidate[i] != '_' && username_candidate[i] != '-') {
            return false;
        }
    }
    return true;
}

/**
 * @brief Reads a request stream to find a username, validates it, and writes a
 * "hello [username]" HTTP response to the response stream.
 *
 * @param request A FILE stream for reading the client request.
 * @param response A FILE stream for writing the HTTP response.
 */
void http_hello_page(FILE* request, FILE* response) {
    // Rule #3: Use a stack-allocated buffer with a safe size.
    char line[MAX_LINE_LENGTH];
    char username[MAX_USERNAME_LENGTH + 1];
    bool valid_username_found = false;

    const char* prefix = "username=";
    size_t prefix_len = strlen(prefix);
    int lines_read = 0;

    // Rule #2, #3: Use fgets for bounded reads and limit total lines read.
    while (!valid_username_found && lines_read < MAX_REQUEST_LINES && fgets(line, sizeof(line), request) != NULL) {
        line[strcspn(line, "\r\n")] = 0; // Strip trailing newline/CR

        if (strncmp(line, prefix, prefix_len) == 0) {
            const char* value_start = line + prefix_len;
            // Rule #1: Validate input *before* copying it to a local buffer.
            if (is_username_valid(value_start)) {
                // Validation passed, now we can safely copy.
                strncpy(username, value_start, sizeof(username) - 1);
                username[sizeof(username) - 1] = '\0'; // Ensure null-termination
                valid_username_found = true;
            }
            // Stop processing after finding the username line, whether valid or not.
            break; 
        }
        lines_read++;
    }

    if (valid_username_found) {
        // Rule #3, #7: Use snprintf for safe, bounded string formatting.
        char body[sizeof("hello ") + MAX_USERNAME_LENGTH];
        snprintf(body, sizeof(body), "hello %s", username);
        
        fprintf(response, "HTTP/1.1 200 OK\r\n");
        fprintf(response, "Content-Type: text/plain; charset=utf-8\r\n");
        fprintf(response, "Content-Length: %zu\r\n", strlen(body));
        fprintf(response, "\r\n");
        fprintf(response, "%s", body);
    } else {
        const char* body = "Bad Request: Invalid or missing username.";
        fprintf(response, "HTTP/1.1 400 Bad Request\r\n");
        fprintf(response, "Content-Type: text/plain; charset=utf-8\r\n");
        fprintf(response, "Content-Length: %zu\r\n", strlen(body));
        fprintf(response, "\r\n");
        fprintf(response, "%s", body);
    }
    fflush(response);
}


#if defined(__GNUC__) || defined(__clang__)
// Test harness using GNU extensions for in-memory streams.
void test(const char* testName, const char* requestPayload) {
    printf("--- Test Case: %s ---\n", testName);

    char* response_buf = NULL;
    size_t response_size = 0;
    FILE* request_stream = fmemopen((void*)requestPayload, strlen(requestPayload), "r");
    FILE* response_stream = open_memstream(&response_buf, &response_size);

    if (!request_stream || !response_stream) {
        perror("Failed to create memory streams");
        if (request_stream) fclose(request_stream);
        if (response_stream) fclose(response_stream);
        if (response_buf) free(response_buf);
        return;
    }

    http_hello_page(request_stream, response_stream);

    fclose(response_stream);
    fclose(request_stream);

    printf("Request:\n%s", requestPayload);
    if (strlen(requestPayload) > 0 && requestPayload[strlen(requestPayload)-1] != '\n') {
        printf("\n");
    }
    printf("Response:\n%s\n\n", response_buf ? response_buf : "[EMPTY]");

    free(response_buf);
}
#else
// Fallback test harness for non-GNU systems (e.g., Windows/MSVC) using temporary files.
void test(const char* testName, const char* requestPayload) {
    printf("--- Test Case: %s (using tmpfile) ---\n", testName);
    
    FILE *request_tmp = tmpfile();
    FILE *response_tmp = tmpfile();
    
    if (!request_tmp || !response_tmp) {
        perror("Could not create temporary files");
        return;
    }
    
    fputs(requestPayload, request_tmp);
    rewind(request_tmp);
    
    http_hello_page(request_tmp, response_tmp);
    
    rewind(response_tmp);
    
    printf("Request:\n%s", requestPayload);
    if (strlen(requestPayload) > 0 && requestPayload[strlen(requestPayload)-1] != '\n') {
        printf("\n");
    }

    printf("Response:\n");
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), response_tmp)) {
        printf("%s", buffer);
    }
    printf("\n\n");
    
    fclose(request_tmp);
    fclose(response_tmp);
}
#endif

int main() {
    test("Valid", "username=Alice\n");

    char long_username_payload[200];
    // Create a username that is one character too long
    snprintf(long_username_payload, sizeof(long_username_payload), "username=%s\n", 
             "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"); // 51 'a's
    test("Long Username", long_username_payload);
    
    test("Invalid Characters", "username=<script>alert('xss')</script>\n");
    test("No Username", "someotherheader=value\n");
    test("Empty Username", "username=\n");

    return 0;
}