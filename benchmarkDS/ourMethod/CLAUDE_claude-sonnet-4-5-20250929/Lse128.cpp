
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LENGTH 100
#define MAX_REQUEST_SIZE 8192
#define MAX_LINE_LENGTH 2048

/* Validate that username contains only alphanumeric, underscore, hyphen */
static bool is_valid_username_char(char c) {
    return isalnum((unsigned char)c) || c == '_' || c == '-';
}

/* Sanitize and validate username input */
static bool sanitize_username(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        return false;
    }
    
    size_t len = strlen(input);
    
    /* Enforce length limit */
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    /* Validate each character and copy to output */
    size_t j = 0;
    for (size_t i = 0; i < len && j < output_size - 1; i++) {
        if (!is_valid_username_char(input[i])) {
            return false;
        }
        output[j++] = input[i];
    }
    output[j] = '\\0';
    
    return j > 0;
}

/* HTML encode a string to prevent XSS */
static void html_encode(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; input[i] && j < output_size - 1; i++) {
        switch (input[i]) {
            case '<':
                if (j + 4 < output_size) {
                    memcpy(output + j, "&lt;", 4);
                    j += 4;
                }
                break;
            case '>':
                if (j + 4 < output_size) {
                    memcpy(output + j, "&gt;", 4);
                    j += 4;
                }
                break;
            case '&':
                if (j + 5 < output_size) {
                    memcpy(output + j, "&amp;", 5);
                    j += 5;
                }
                break;
            case '"':
                if (j + 6 < output_size) {
                    memcpy(output + j, "&quot;", 6);
                    j += 6;
                }
                break;
            case '\\'':
                if (j + 6 < output_size) {
                    memcpy(output + j, "&#x27;", 6);
                    j += 6;
                }
                break;
            default:
                if (j < output_size - 1) {
                    output[j++] = input[i];
                }
        }
    }
    output[j] = '\\0';
}

/* Basic URL decode */
static void url_decode(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) {
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; input[i] && j < output_size - 1; i++) {
        if (input[i] == '%' && input[i+1] && input[i+2]) {
            /* Simple hex decode */
            int hex_val;
            if (sscanf(input + i + 1, "%2x", &hex_val) == 1) {
                output[j++] = (char)hex_val;
                i += 2;
            } else {
                output[j++] = input[i];
            }
        } else if (input[i] == '+') {
            output[j++] = ' ';
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void http_hello_page(FILE* request, FILE* response) {
    if (!request || !response) {
        return;
    }
    
    char line[MAX_LINE_LENGTH];
    char username[MAX_USERNAME_LENGTH + 1] = {0};
    char sanitized[MAX_USERNAME_LENGTH + 1] = {0};
    char encoded[MAX_USERNAME_LENGTH * 6 + 1] = {0};
    bool found_username = false;
    size_t bytes_read = 0;
    
    /* Read request headers with size limit */
    while (bytes_read < MAX_REQUEST_SIZE && fgets(line, sizeof(line), request)) {
        bytes_read += strlen(line);
        
        /* Remove trailing newline */
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\\n' || line[len-1] == '\\r')) {
            line[--len] = '\\0';
        }
        
        /* Empty line indicates end of headers */
        if (len == 0) {
            break;
        }
        
        /* Parse GET request line */
        if (strncmp(line, "GET ", 4) == 0) {
            char* query_start = strchr(line + 4, '?');
            if (query_start) {
                char* space = strchr(query_start, ' ');
                if (space) {
                    *space = '\\0';
                }
                
                /* Look for username parameter */
                char* param = strstr(query_start, "username=");
                if (param) {
                    param += 9; /* Length of "username=" */
                    char* param_end = strchr(param, '&');
                    if (param_end) {
                        *param_end = '\\0';
                    }
                    
                    /* URL decode and copy username */
                    url_decode(param, username, sizeof(username));
                    found_username = true;
                    break;
                }
            }
        }
    }
    
    /* Validate and sanitize username */
    if (found_username && sanitize_username(username, sanitized, sizeof(sanitized))) {
        /* HTML encode for safe output */
        html_encode(sanitized, encoded, sizeof(encoded));
        
        /* Write HTTP response with security headers */
        fprintf(response, "HTTP/1.1 200 OK\\r\\n");
        fprintf(response, "Content-Type: text/html; charset=UTF-8\\r\\n");
        fprintf(response, "X-Content-Type-Options: nosniff\\r\\n");
        fprintf(response, "X-Frame-Options: DENY\\r\\n");
        fprintf(response, "Cache-Control: no-store\\r\\n");
        fprintf(response, "\\r\\n");
        fprintf(response, "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\"></head><body>");
        fprintf(response, "hello %s", encoded);
        fprintf(response, "</body></html>");
    } else {
        /* Return generic error message */
        fprintf(response, "HTTP/1.1 400 Bad Request\\r\\n");
        fprintf(response, "Content-Type: text/plain; charset=UTF-8\\r\\n");
        fprintf(response, "\\r\\n");
        fprintf(response, "Invalid username");
    }
    
    fflush(response);
}

int main(void) {
    const char* test_cases[][2] = {
        {"GET /?username=john HTTP/1.1\\r\\n\\r\\n", "Valid username"},
        {"GET /?username=john<script> HTTP/1.1\\r\\n\\r\\n", "XSS attempt"},
        {"GET /?username=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa HTTP/1.1\\r\\n\\r\\n", "Long username"},
        {"GET /?username=john_doe HTTP/1.1\\r\\n\\r\\n", "Username with underscore"},
        {"GET / HTTP/1.1\\r\\n\\r\\n", "No username"}
    };
    
    for (size_t i = 0; i < 5; i++) {
        printf("\\n=== Test: %s ===\\n", test_cases[i][1]);
        
        /* Create temporary files for request and response */
        FILE* req = tmpfile();
        FILE* resp = tmpfile();
        
        if (req && resp) {
            /* Write test request */
            fputs(test_cases[i][0], req);
            rewind(req);
            
            /* Process request */
            http_hello_page(req, resp);
            
            /* Read and print response */
            rewind(resp);
            char buffer[1024];
            while (fgets(buffer, sizeof(buffer), resp)) {
                printf("%s", buffer);
            }
            printf("\\n");
            
            fclose(req);
            fclose(resp);
        }
    }
    
    return 0;
}
