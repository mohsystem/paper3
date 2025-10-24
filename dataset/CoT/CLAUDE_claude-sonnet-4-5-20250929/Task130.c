
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_HEADERS 10
#define MAX_HEADER_LENGTH 256
#define MAX_ORIGINS 10
#define MAX_METHODS 10
#define MAX_AGE 3600

typedef struct {
    char key[MAX_HEADER_LENGTH];
    char value[MAX_HEADER_LENGTH];
} Header;

typedef struct {
    bool allowed;
    Header headers[MAX_HEADERS];
    int header_count;
    char message[MAX_HEADER_LENGTH];
} CORSResponse;

static const char* ALLOWED_ORIGINS[] = {
    "https://example.com",
    "https://app.example.com",
    "http://localhost:3000"
};
static const int ALLOWED_ORIGINS_COUNT = 3;

static const char* ALLOWED_METHODS[] = {
    "GET", "POST", "PUT", "DELETE", "OPTIONS"
};
static const int ALLOWED_METHODS_COUNT = 5;

static const char* ALLOWED_HEADERS[] = {
    "Content-Type", "Authorization", "X-Requested-With"
};
static const int ALLOWED_HEADERS_COUNT = 3;

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = 0;
    
    memmove(str, start, end - start + 2);
}

void to_upper(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void to_lower(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

bool sanitize_header(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        return false;
    }
    
    size_t j = 0;
    for (size_t i = 0; input[i] && j < output_size - 1; i++) {
        unsigned char c = (unsigned char)input[i];
        // Remove control characters
        if (c >= 32 && c != 127) {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
    
    trim(output);
    
    // Check for CRLF injection
    if (strchr(output, '\\r') || strchr(output, '\\n')) {
        return false;
    }
    
    return strlen(output) > 0;
}

bool is_origin_allowed(const char* origin) {
    for (int i = 0; i < ALLOWED_ORIGINS_COUNT; i++) {
        if (strcmp(origin, ALLOWED_ORIGINS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool is_method_allowed(const char* method) {
    char upper_method[MAX_HEADER_LENGTH];
    strncpy(upper_method, method, MAX_HEADER_LENGTH - 1);
    upper_method[MAX_HEADER_LENGTH - 1] = '\\0';
    to_upper(upper_method);
    
    for (int i = 0; i < ALLOWED_METHODS_COUNT; i++) {
        if (strcmp(upper_method, ALLOWED_METHODS[i]) == 0) {
            return true;
        }
    }
    return false;
}

bool validate_requested_headers(const char* requested_headers) {
    if (requested_headers == NULL || strlen(requested_headers) == 0) {
        return true;
    }
    
    char headers_copy[MAX_HEADER_LENGTH];
    strncpy(headers_copy, requested_headers, MAX_HEADER_LENGTH - 1);
    headers_copy[MAX_HEADER_LENGTH - 1] = '\\0';
    
    char* token = strtok(headers_copy, ",");
    while (token != NULL) {
        trim(token);
        to_lower(token);
        
        if (strlen(token) > 0) {
            bool found = false;
            for (int i = 0; i < ALLOWED_HEADERS_COUNT; i++) {
                char allowed_lower[MAX_HEADER_LENGTH];
                strncpy(allowed_lower, ALLOWED_HEADERS[i], MAX_HEADER_LENGTH - 1);
                allowed_lower[MAX_HEADER_LENGTH - 1] = '\\0';
                to_lower(allowed_lower);
                
                if (strcmp(token, allowed_lower) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                return false;
            }
        }
        token = strtok(NULL, ",");
    }
    return true;
}

void add_header(CORSResponse* response, const char* key, const char* value) {
    if (response->header_count < MAX_HEADERS) {
        strncpy(response->headers[response->header_count].key, key, MAX_HEADER_LENGTH - 1);
        response->headers[response->header_count].key[MAX_HEADER_LENGTH - 1] = '\\0';
        strncpy(response->headers[response->header_count].value, value, MAX_HEADER_LENGTH - 1);
        response->headers[response->header_count].value[MAX_HEADER_LENGTH - 1] = '\\0';
        response->header_count++;
    }
}

CORSResponse handle_cors(const char* origin, const char* method, const char* request_headers) {
    CORSResponse response;
    response.allowed = false;
    response.header_count = 0;
    memset(response.message, 0, MAX_HEADER_LENGTH);
    
    // Sanitize origin
    char sanitized_origin[MAX_HEADER_LENGTH];
    if (!sanitize_header(origin, sanitized_origin, MAX_HEADER_LENGTH)) {
        strncpy(response.message, "Invalid origin", MAX_HEADER_LENGTH - 1);
        return response;
    }
    
    // Check if origin is allowed
    if (!is_origin_allowed(sanitized_origin)) {
        strncpy(response.message, "Origin not allowed", MAX_HEADER_LENGTH - 1);
        return response;
    }
    
    // Sanitize method
    char sanitized_method[MAX_HEADER_LENGTH];
    if (!sanitize_header(method, sanitized_method, MAX_HEADER_LENGTH)) {
        strncpy(response.message, "Invalid method", MAX_HEADER_LENGTH - 1);
        return response;
    }
    
    // Check if method is allowed
    if (!is_method_allowed(sanitized_method)) {
        strncpy(response.message, "Method not allowed", MAX_HEADER_LENGTH - 1);
        return response;
    }
    
    // Set CORS headers
    add_header(&response, "Access-Control-Allow-Origin", sanitized_origin);
    add_header(&response, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    add_header(&response, "Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    add_header(&response, "Access-Control-Max-Age", "3600");
    add_header(&response, "Access-Control-Allow-Credentials", "true");
    
    // Security headers
    add_header(&response, "X-Content-Type-Options", "nosniff");
    add_header(&response, "X-Frame-Options", "DENY");
    add_header(&response, "Strict-Transport-Security", "max-age=31536000; includeSubDomains");
    
    // Handle preflight
    char upper_method[MAX_HEADER_LENGTH];
    strncpy(upper_method, sanitized_method, MAX_HEADER_LENGTH - 1);
    upper_method[MAX_HEADER_LENGTH - 1] = '\\0';
    to_upper(upper_method);
    
    if (strcmp(upper_method, "OPTIONS") == 0) {
        if (request_headers != NULL && strlen(request_headers) > 0) {
            char sanitized_req_headers[MAX_HEADER_LENGTH];
            if (sanitize_header(request_headers, sanitized_req_headers, MAX_HEADER_LENGTH)) {
                if (!validate_requested_headers(sanitized_req_headers)) {
                    strncpy(response.message, "Requested headers not allowed", MAX_HEADER_LENGTH - 1);
                    return response;
                }
            }
        }
        response.allowed = true;
        strncpy(response.message, "Preflight request approved", MAX_HEADER_LENGTH - 1);
        return response;
    }
    
    response.allowed = true;
    strncpy(response.message, "CORS request approved", MAX_HEADER_LENGTH - 1);
    return response;
}

void print_cors_response(const CORSResponse* response) {
    printf("Allowed: %s\\n", response->allowed ? "true" : "false");
    printf("Message: %s\\n", response->message);
    printf("Headers:\\n");
    for (int i = 0; i < response->header_count; i++) {
        printf("  %s: %s\\n", response->headers[i].key, response->headers[i].value);
    }
    printf("\\n");
}

int main() {
    printf("=== CORS Implementation Test Cases ===\\n\\n");
    
    // Test case 1: Valid GET request
    printf("Test 1: Valid GET request from allowed origin\\n");
    CORSResponse resp1 = handle_cors("https://example.com", "GET", NULL);
    print_cors_response(&resp1);
    
    // Test case 2: Valid OPTIONS preflight request
    printf("Test 2: Valid OPTIONS preflight request\\n");
    CORSResponse resp2 = handle_cors("https://app.example.com", "OPTIONS", "Content-Type, Authorization");
    print_cors_response(&resp2);
    
    // Test case 3: Invalid origin
    printf("Test 3: Request from non-allowed origin\\n");
    CORSResponse resp3 = handle_cors("https://malicious.com", "GET", NULL);
    print_cors_response(&resp3);
    
    // Test case 4: Invalid method
    printf("Test 4: Request with non-allowed method\\n");
    CORSResponse resp4 = handle_cors("https://example.com", "TRACE", NULL);
    print_cors_response(&resp4);
    
    // Test case 5: Valid POST request from localhost
    printf("Test 5: Valid POST request from localhost\\n");
    CORSResponse resp5 = handle_cors("http://localhost:3000", "POST", "Content-Type");
    print_cors_response(&resp5);
    
    return 0;
}
