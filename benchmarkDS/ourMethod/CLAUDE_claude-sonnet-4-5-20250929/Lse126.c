
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Maximum request body size to prevent resource exhaustion */
#define MAX_REQUEST_BODY_SIZE 8192
/* Maximum username length */
#define MAX_USERNAME_LENGTH 256
/* Buffer size for HTML output */
#define OUTPUT_BUFFER_SIZE 16384

/* HTTP request structure */
typedef struct {
    char method[16];
    char path[256];
    char* body;
    size_t body_length;
} HttpRequest;

/* HTTP response structure */
typedef struct {
    int status_code;
    char* headers;
    char* body;
    size_t body_length;
} HttpResponse;

/* Initialize response structure */
void init_response(HttpResponse* response) {
    if (response == NULL) return;
    response->status_code = 0;
    response->headers = NULL;
    response->body = NULL;
    response->body_length = 0;
}

/* Free response resources */
void free_response(HttpResponse* response) {
    if (response == NULL) return;
    if (response->headers != NULL) {
        free(response->headers);
        response->headers = NULL;
    }
    if (response->body != NULL) {
        free(response->body);
        response->body = NULL;
    }
}

/* HTML entity encoding to prevent XSS */
char* sanitize_html(const char* input) {
    if (input == NULL) return NULL;
    
    size_t input_len = strlen(input);
    /* Allocate worst case: every character could become 6 chars (&#x27;) */
    size_t max_output_len = input_len * 6 + 1;
    char* output = (char*)calloc(max_output_len, sizeof(char));
    if (output == NULL) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < input_len && j < max_output_len - 7; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&output[j], "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
    return output;
}

/* Trim whitespace from string (modifies in place) */
void trim(char* str) {
    if (str == NULL) return;
    
    /* Trim leading whitespace */
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    /* Trim trailing whitespace */
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\\0';
    
    /* Move trimmed string to beginning if needed */
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

/* Function to handle /hello endpoint */
HttpResponse http_hello_page(const HttpRequest* request) {
    HttpResponse response;
    init_response(&response);
    
    /* Validate request pointer */
    if (request == NULL) {
        response.status_code = 400;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Invalid Request");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Only accept POST requests */
    if (strcmp(request->method, "POST") != 0) {
        response.status_code = 405;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Method Not Allowed");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Validate request body size */
    if (request->body_length > MAX_REQUEST_BODY_SIZE) {
        response.status_code = 413;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Request Entity Too Large");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Validate request body exists */
    if (request->body == NULL || request->body_length == 0) {
        response.status_code = 400;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Invalid Request");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Copy username with bounds checking */
    char username[MAX_USERNAME_LENGTH + 1];
    size_t copy_len = request->body_length < MAX_USERNAME_LENGTH ? 
                      request->body_length : MAX_USERNAME_LENGTH;
    memcpy(username, request->body, copy_len);
    username[copy_len] = '\\0';
    
    /* Trim whitespace */
    trim(username);
    
    /* Validate username not empty */
    if (strlen(username) == 0) {
        response.status_code = 400;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Invalid Request");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Sanitize username to prevent XSS */
    char* username_safe = sanitize_html(username);
    if (username_safe == NULL) {
        response.status_code = 500;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Internal Server Error");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Construct response HTML with bounds checking */
    char* outputbuf = (char*)calloc(OUTPUT_BUFFER_SIZE, sizeof(char));
    if (outputbuf == NULL) {
        free(username_safe);
        response.status_code = 500;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Internal Server Error");
        response.body_length = strlen(response.body);
        return response;
    }
    
    int written = snprintf(outputbuf, OUTPUT_BUFFER_SIZE,
        "<!DOCTYPE html>\\n"
        "<html lang=\\"en\\">\\n"
        "<head>\\n"
        "<meta charset=\\"UTF-8\\">\\n"
        "<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
        "<title>Hello Page</title>\\n"
        "</head>\\n"
        "<body>\\n"
        "<h1>Hello, %s!</h1>\\n"
        "</body>\\n"
        "</html>", username_safe);
    
    free(username_safe);
    username_safe = NULL;
    
    /* Check if output was truncated */
    if (written < 0 || written >= OUTPUT_BUFFER_SIZE) {
        free(outputbuf);
        response.status_code = 500;
        response.headers = strdup("Content-Type: text/plain; charset=UTF-8\\r\\n");
        response.body = strdup("Internal Server Error");
        response.body_length = strlen(response.body);
        return response;
    }
    
    /* Set response with security headers */
    response.status_code = 200;
    response.headers = strdup(
        "Content-Type: text/html; charset=UTF-8\\r\\n"
        "X-Content-Type-Options: nosniff\\r\\n"
        "X-Frame-Options: DENY\\r\\n"
        "Content-Security-Policy: default-src 'self'\\r\\n"
    );
    response.body = outputbuf;
    response.body_length = strlen(outputbuf);
    
    return response;
}

/* Test function */
void test_http_hello_page(const char* username) {
    HttpRequest request;
    memset(&request, 0, sizeof(HttpRequest));
    
    strncpy(request.method, "POST", sizeof(request.method) - 1);
    request.method[sizeof(request.method) - 1] = '\\0';
    
    strncpy(request.path, "/hello", sizeof(request.path) - 1);
    request.path[sizeof(request.path) - 1] = '\\0';
    
    if (username != NULL) {
        request.body_length = strlen(username);
        request.body = (char*)malloc(request.body_length + 1);
        if (request.body != NULL) {
            memcpy(request.body, username, request.body_length);
            request.body[request.body_length] = '\\0';
        }
    }
    
    HttpResponse response = http_hello_page(&request);
    
    printf("Test username: %.50s\\n", username ? username : "(null)");
    printf("Response code: %d\\n", response.status_code);
    if (response.body != NULL) {
        printf("%s\\n", response.body);
    }
    printf("\\n");
    
    /* Cleanup */
    if (request.body != NULL) {
        free(request.body);
    }
    free_response(&response);
}

int main(void) {
    /* Test cases */
    test_http_hello_page("Alice");
    test_http_hello_page("Bob123");
    test_http_hello_page("<script>alert('xss')</script>");
    
    /* Long username test */
    char* long_username = (char*)malloc(301);
    if (long_username != NULL) {
        memset(long_username, 'A', 300);
        long_username[300] = '\\0';
        test_http_hello_page(long_username);
        free(long_username);
    }
    
    test_http_hello_page(""); /* Empty username */
    
    return 0;
}
