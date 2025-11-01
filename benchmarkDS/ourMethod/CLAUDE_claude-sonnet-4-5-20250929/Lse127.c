
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Maximum allowed lengths to prevent resource exhaustion */
#define MAX_USERNAME_LENGTH 100
#define MAX_REQUEST_SIZE 1024

/**
 * Sanitizes username to prevent injection attacks
 * Allows only alphanumeric characters, spaces, hyphens, and underscores
 * @param username The username to sanitize
 * @param output Buffer to store sanitized username
 * @param output_size Size of output buffer
 */
void sanitize_username(const char* username, char* output, size_t output_size) {
    if (username == NULL || output == NULL || output_size == 0) {
        if (output != NULL && output_size > 0) {
            strncpy(output, "Guest", output_size - 1);
            output[output_size - 1] = '\\0';
        }
        return;
    }
    
    size_t j = 0;
    size_t username_len = strnlen(username, MAX_USERNAME_LENGTH + 1);
    
    /* Copy only safe characters */
    for (size_t i = 0; i < username_len && j < output_size - 1; i++) {
        unsigned char c = (unsigned char)username[i];
        if (isalnum(c) || c == ' ' || c == '-' || c == '_') {
            output[j++] = username[i];
        }
    }
    output[j] = '\\0';
    
    /* Trim whitespace */
    size_t start = 0;
    while (start < j && isspace((unsigned char)output[start])) {
        start++;
    }
    
    if (start > 0) {
        memmove(output, output + start, j - start + 1);
        j -= start;
    }
    
    while (j > 0 && isspace((unsigned char)output[j - 1])) {
        j--;
    }
    output[j] = '\\0';
    
    /* If empty after sanitization, use default */
    if (output[0] == '\\0') {
        strncpy(output, "Guest", output_size - 1);
        output[output_size - 1] = '\\0';
    }
}

/**
 * Extracts username from request string
 * @param request The request string
 * @param username Buffer to store extracted username
 * @param username_size Size of username buffer
 */
void extract_username(const char* request, char* username, size_t username_size) {
    if (request == NULL || username == NULL || username_size == 0) {
        if (username != NULL && username_size > 0) {
            strncpy(username, "Guest", username_size - 1);
            username[username_size - 1] = '\\0';
        }
        return;
    }
    
    /* Look for username parameter */
    const char* pos = strstr(request, "username=");
    if (pos == NULL) {
        strncpy(username, "Guest", username_size - 1);
        username[username_size - 1] = '\\0';
        return;
    }
    
    /* Move past "username=" */
    pos += 9;
    
    /* Copy until delimiter or end */
    size_t i = 0;
    while (pos[i] != '\\0' && pos[i] != '&' && pos[i] != '\\n' && 
           pos[i] != '\\r' && i < username_size - 1) {
        username[i] = pos[i];
        i++;
    }
    username[i] = '\\0';
}

/**
 * Processes HTTP request and sends greeting response
 * @param request Input stream containing the client request
 * @param response Output stream for sending response to client
 */
void http_hello_page(FILE* request, FILE* response) {
    if (request == NULL || response == NULL) {
        fprintf(stderr, "Error: Request and response streams cannot be NULL\\n");
        return;
    }
    
    /* Read request with size limit to prevent memory exhaustion */
    char* buffer = (char*)calloc(MAX_REQUEST_SIZE + 1, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return;
    }
    
    /* Read with bounds checking */
    size_t bytes_read = fread(buffer, 1, MAX_REQUEST_SIZE, request);
    buffer[bytes_read] = '\\0'; /* Ensure null termination */
    
    /* Extract username */
    char raw_username[MAX_USERNAME_LENGTH + 1];
    extract_username(buffer, raw_username, sizeof(raw_username));
    
    /* Sanitize username to prevent injection attacks */
    char username[MAX_USERNAME_LENGTH + 1];
    sanitize_username(raw_username, username, sizeof(username));
    
    /* Construct response with sanitized data */
    char response_message[MAX_USERNAME_LENGTH + 20];
    int written = snprintf(response_message, sizeof(response_message), "Hello %s", username);
    
    /* Validate snprintf result */
    if (written < 0 || (size_t)written >= sizeof(response_message)) {
        fprintf(stderr, "Error: Response construction failed\\n");
        /* Clear sensitive data before free */
        explicit_bzero(buffer, MAX_REQUEST_SIZE + 1);
        free(buffer);
        return;
    }
    
    /* Write response */
    size_t msg_len = strnlen(response_message, sizeof(response_message));
    fwrite(response_message, 1, msg_len, response);
    fflush(response);
    
    /* Clear sensitive data before free */
    explicit_bzero(buffer, MAX_REQUEST_SIZE + 1);
    free(buffer);
}

int main(void) {
    /* Test case 1: Normal username */
    {
        FILE* req = fmemopen("username=Alice", 14, "r");
        FILE* resp = tmpfile();
        if (req && resp) {
            http_hello_page(req, resp);
            rewind(resp);
            char result[256];
            if (fgets(result, sizeof(result), resp)) {
                printf("Test 1: %s\\n", result);
            }
            fclose(req);
            fclose(resp);
        }
    }
    
    /* Test case 2: Username with special characters (should be sanitized) */
    {
        FILE* req = fmemopen("username=Bob<script>", 20, "r");
        FILE* resp = tmpfile();
        if (req && resp) {
            http_hello_page(req, resp);
            rewind(resp);
            char result[256];
            if (fgets(result, sizeof(result), resp)) {
                printf("Test 2: %s\\n", result);
            }
            fclose(req);
            fclose(resp);
        }
    }
    
    /* Test case 3: No username provided */
    {
        FILE* req = fmemopen("other=data", 10, "r");
        FILE* resp = tmpfile();
        if (req && resp) {
            http_hello_page(req, resp);
            rewind(resp);
            char result[256];
            if (fgets(result, sizeof(result), resp)) {
                printf("Test 3: %s\\n", result);
            }
            fclose(req);
            fclose(resp);
        }
    }
    
    /* Test case 4: Very long username (should be truncated) */
    {
        char long_request[300];
        snprintf(long_request, sizeof(long_request), "username=%s", 
                 "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        FILE* req = fmemopen(long_request, strlen(long_request), "r");
        FILE* resp = tmpfile();
        if (req && resp) {
            http_hello_page(req, resp);
            rewind(resp);
            char result[256];
            if (fgets(result, sizeof(result), resp)) {
                printf("Test 4: %s\\n", result);
            }
            fclose(req);
            fclose(resp);
        }
    }
    
    /* Test case 5: Username with valid special characters */
    {
        FILE* req = fmemopen("username=John_Doe-123", 21, "r");
        FILE* resp = tmpfile();
        if (req && resp) {
            http_hello_page(req, resp);
            rewind(resp);
            char result[256];
            if (fgets(result, sizeof(result), resp)) {
                printf("Test 5: %s\\n", result);
            }
            fclose(req);
            fclose(resp);
        }
    }
    
    return 0;
}
