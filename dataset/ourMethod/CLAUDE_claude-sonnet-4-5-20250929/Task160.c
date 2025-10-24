
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>
#include <arpa/inet.h>

#define MAX_URL_LENGTH 2048
#define MAX_RESPONSE_SIZE 1048576  // 1MB limit
#define TIMEOUT_SECONDS 30
#define MAX_REDIRECTS 5

// Structure to hold response data with size tracking
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

// Secure zeroization function that compiler cannot optimize away
static void explicit_zero(void *ptr, size_t len) {
    volatile unsigned char *p = ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Initialize response buffer with security checks
static int init_response_buffer(ResponseBuffer *buffer) {
    if (!buffer) return 0;
    
    buffer->capacity = 4096;  // Start with 4KB
    buffer->data = calloc(1, buffer->capacity);
    if (!buffer->data) return 0;
    
    buffer->size = 0;
    return 1;
}

// Free response buffer with secure cleanup
static void free_response_buffer(ResponseBuffer *buffer) {
    if (!buffer) return;
    if (buffer->data) {
        explicit_zero(buffer->data, buffer->capacity);
        free(buffer->data);
        buffer->data = NULL;
    }
    buffer->size = 0;
    buffer->capacity = 0;
}

// Callback for libcurl to write response data with bounds checking
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = 0;
    ResponseBuffer *buffer = (ResponseBuffer *)userp;
    
    // Check for integer overflow before multiplication
    if (nmemb > 0 && size > SIZE_MAX / nmemb) {
        return 0;  // Signal error to libcurl
    }
    realsize = size * nmemb;
    
    // Check if adding realsize would overflow or exceed max size
    if (realsize > MAX_RESPONSE_SIZE - buffer->size) {
        return 0;  // Reject oversized response
    }
    
    // Grow buffer if needed, with maximum limit
    if (buffer->size + realsize + 1 > buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        
        // Ensure new capacity can hold the data and doesn't overflow\n        while (new_capacity < buffer->size + realsize + 1) {\n            if (new_capacity > MAX_RESPONSE_SIZE / 2) {\n                return 0;  // Would exceed maximum\n            }\n            new_capacity *= 2;\n        }\n        \n        if (new_capacity > MAX_RESPONSE_SIZE) {\n            new_capacity = MAX_RESPONSE_SIZE;\n        }\n        \n        char *new_data = realloc(buffer->data, new_capacity);\n        if (!new_data) {\n            return 0;  // Allocation failed\n        }\n        \n        buffer->data = new_data;\n        buffer->capacity = new_capacity;\n    }\n    \n    // Safe copy with bounds check already performed\n    memcpy(buffer->data + buffer->size, contents, realsize);\n    buffer->size += realsize;\n    buffer->data[buffer->size] = '\\0';  // Null terminate\n    \n    return realsize;\n}\n\n// Validate URL format and security requirements\nstatic int validate_url(const char *url) {\n    if (!url) return 0;\n    \n    size_t len = strlen(url);\n    \n    // Check length bounds\n    if (len == 0 || len > MAX_URL_LENGTH) {\n        return 0;\n    }\n    \n    // Must start with https:// (enforce encrypted connections only)\n    if (strncmp(url, "https://", 8) != 0) {\n        return 0;\n    }\n    \n    // Check for null bytes (防止截断攻击)\n    if (memchr(url, '\\0', len) != url + len) {\n        return 0;\n    }\n    \n    // Basic validation: must contain a domain after https://\n    const char *domain_start = url + 8;\n    if (*domain_start == '\\0' || *domain_start == '/' || *domain_start == ':') {\n        return 0;\n    }\n    \n    // Reject URLs with credentials (user:pass@host)\n    if (strchr(domain_start, '@') != NULL) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n// Check if IP address is private/internal\nstatic int is_private_ip(const char *ip) {\n    struct in_addr addr;\n    \n    if (inet_pton(AF_INET, ip, &addr) != 1) {\n        return 0;  // Invalid IP or IPv6 (we'll reject IPv6 for simplicity)
    }
    
    unsigned char *bytes = (unsigned char *)&addr.s_addr;
    
    // 10.0.0.0/8
    if (bytes[0] == 10) return 1;
    
    // 172.16.0.0/12
    if (bytes[0] == 172 && (bytes[1] >= 16 && bytes[1] <= 31)) return 1;
    
    // 192.168.0.0/16
    if (bytes[0] == 192 && bytes[1] == 168) return 1;
    
    // 127.0.0.0/8 (localhost)
    if (bytes[0] == 127) return 1;
    
    // 169.254.0.0/16 (link-local)
    if (bytes[0] == 169 && bytes[1] == 254) return 1;
    
    // 0.0.0.0/8
    if (bytes[0] == 0) return 1;
    
    return 0;
}

// Make secure HTTP request with all protections enabled
static int make_secure_request(const char *url, char **response_out) {
    CURL *curl = NULL;
    CURLcode res;
    ResponseBuffer buffer = {NULL, 0, 0};
    int success = 0;
    
    if (!url || !response_out) return 0;
    *response_out = NULL;
    
    // Validate URL before any network activity
    if (!validate_url(url)) {
        fprintf(stderr, "Error: Invalid URL format. Only HTTPS URLs are allowed.\\n");
        return 0;
    }
    
    // Initialize response buffer
    if (!init_response_buffer(&buffer)) {
        fprintf(stderr, "Error: Failed to allocate memory.\\n");
        return 0;
    }
    
    // Initialize libcurl
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize CURL.\\n");
        free_response_buffer(&buffer);
        return 0;
    }
    
    // Set URL - libcurl will parse and validate further
    curl_easy_setopt(curl, CURLOPT_URL, url);
    
    // SECURITY: Restrict to HTTPS protocol only
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    
    // SECURITY: Enable SSL/TLS certificate verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // SECURITY: Require TLS 1.2 or higher
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
    
    // SECURITY: Set timeout to prevent hanging
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    
    // SECURITY: Limit redirects to prevent redirect loops
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRECTS);
    
    // Set write callback with bounds-checked buffer
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    
    // SECURITY: Disable verbose output that might leak information
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
    
    // SECURITY: Set a reasonable user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureHTTPClient/1.0");
    
    // Perform the request
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        // Generic error message - don't leak internal details
        fprintf(stderr, "Error: Request failed.\\n");
        success = 0;
    } else {
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        if (http_code >= 200 && http_code < 300) {
            // Success - transfer ownership of buffer data
            *response_out = buffer.data;
            buffer.data = NULL;  // Prevent free in cleanup
            success = 1;
        } else {
            fprintf(stderr, "Error: HTTP request returned status code %ld.\\n", http_code);
            success = 0;
        }
    }
    
    // Cleanup
    curl_easy_cleanup(curl);
    free_response_buffer(&buffer);
    
    return success;
}

// Main function with test cases
int main(void) {
    char url[MAX_URL_LENGTH];
    char *response = NULL;
    
    // Test case 1: Valid HTTPS URL (example.com)
    printf("Test 1: Valid HTTPS URL\\n");
    snprintf(url, sizeof(url), "https://example.com");
    if (make_secure_request(url, &response)) {
        printf("Success! Response length: %zu bytes\\n", response ? strlen(response) : 0);
        if (response) {
            printf("Response preview: %.100s...\\n", response);
            explicit_zero(response, strlen(response));
            free(response);
            response = NULL;
        }
    }
    printf("\\n");
    
    // Test case 2: Invalid URL (HTTP instead of HTTPS)
    printf("Test 2: Invalid URL (HTTP not allowed)\\n");
    snprintf(url, sizeof(url), "http://example.com");
    if (make_secure_request(url, &response)) {
        printf("Success (unexpected)\\n");
        if (response) {
            explicit_zero(response, strlen(response));
            free(response);
            response = NULL;
        }
    } else {
        printf("Rejected as expected.\\n");
    }
    printf("\\n");
    
    // Test case 3: Empty URL
    printf("Test 3: Empty URL\\n");
    snprintf(url, sizeof(url), "");
    if (make_secure_request(url, &response)) {
        printf("Success (unexpected)\\n");
        if (response) {
            explicit_zero(response, strlen(response));
            free(response);
            response = NULL;
        }
    } else {
        printf("Rejected as expected.\\n");
    }
    printf("\\n");
    
    // Test case 4: URL with credentials (should be rejected)
    printf("Test 4: URL with credentials\\n");
    snprintf(url, sizeof(url), "https://user:pass@example.com");
    if (make_secure_request(url, &response)) {
        printf("Success (unexpected)\\n");
        if (response) {
            explicit_zero(response, strlen(response));
            free(response);
            response = NULL;
        }
    } else {
        printf("Rejected as expected.\\n");
    }
    printf("\\n");
    
    // Test case 5: Valid HTTPS API endpoint
    printf("Test 5: Valid HTTPS API endpoint\\n");
    snprintf(url, sizeof(url), "https://api.github.com");
    if (make_secure_request(url, &response)) {
        printf("Success! Response length: %zu bytes\\n", response ? strlen(response) : 0);
        if (response) {
            explicit_zero(response, strlen(response));
            free(response);
            response = NULL;
        }
    }
    printf("\\n");
    
    return 0;
}
