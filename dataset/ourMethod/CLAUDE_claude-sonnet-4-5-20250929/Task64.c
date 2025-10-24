
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Maximum allowed response size to prevent memory exhaustion (10 MB)
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024)
// Initial buffer size
#define INITIAL_BUFFER_SIZE 4096

// Structure to hold response data
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

// Initialize response buffer with secure defaults
static int init_response_buffer(ResponseBuffer *buf) {
    if (!buf) return 0;
    
    buf->data = malloc(INITIAL_BUFFER_SIZE);
    if (!buf->data) {
        return 0;
    }
    buf->size = 0;
    buf->capacity = INITIAL_BUFFER_SIZE;
    buf->data[0] = '\\0';
    return 1;
}

// Free response buffer
static void free_response_buffer(ResponseBuffer *buf) {
    if (buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
        buf->size = 0;
        buf->capacity = 0;
    }
}

// Write callback for libcurl - handles incoming data with bounds checking
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize;
    ResponseBuffer *buf = (ResponseBuffer *)userp;
    
    if (!contents || !buf) return 0;
    
    // Check for integer overflow in multiplication
    if (nmemb > 0 && size > SIZE_MAX / nmemb) {
        return 0; // Overflow would occur
    }
    realsize = size * nmemb;
    
    // Check if adding realsize would exceed maximum allowed size
    if (buf->size > MAX_RESPONSE_SIZE || realsize > MAX_RESPONSE_SIZE - buf->size) {
        return 0; // Would exceed maximum size
    }
    
    // Grow buffer if needed, with bounds checking
    if (buf->size + realsize + 1 > buf->capacity) {
        size_t new_capacity = buf->capacity * 2;
        
        // Ensure new capacity is sufficient and doesn't overflow\n        while (new_capacity < buf->size + realsize + 1) {\n            if (new_capacity > SIZE_MAX / 2) {\n                return 0; // Cannot grow further\n            }\n            new_capacity *= 2;\n        }\n        \n        // Cap at maximum allowed size\n        if (new_capacity > MAX_RESPONSE_SIZE) {\n            new_capacity = MAX_RESPONSE_SIZE;\n        }\n        \n        char *new_data = realloc(buf->data, new_capacity);\n        if (!new_data) {\n            return 0; // Allocation failed\n        }\n        buf->data = new_data;\n        buf->capacity = new_capacity;\n    }\n    \n    // Safe to copy now - bounds verified above\n    memcpy(buf->data + buf->size, contents, realsize);\n    buf->size += realsize;\n    buf->data[buf->size] = '\\0'; // Null terminate\n    \n    return realsize;\n}\n\n// Validate URL format - ensure it starts with https://\nstatic int validate_url(const char *url) {\n    if (!url) return 0;\n    \n    size_t len = strlen(url);\n    if (len < 8 || len > 2048) return 0; // Reasonable URL length limits\n    \n    // Enforce HTTPS only for security\n    if (strncmp(url, "https://", 8) != 0) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n// Basic JSON validation - checks for balanced braces/brackets\nstatic int validate_json_structure(const char *json) {\n    if (!json) return 0;\n    \n    int brace_count = 0;\n    int bracket_count = 0;\n    int in_string = 0;\n    int escape_next = 0;\n    \n    for (const char *p = json; *p; p++) {\n        if (escape_next) {\n            escape_next = 0;\n            continue;\n        }\n        \n        if (*p == '\\\\') {\n            escape_next = 1;\n            continue;\n        }\n        \n        if (*p == '"') {\n            in_string = !in_string;\n            continue;\n        }\n        \n        if (in_string) continue;\n        \n        if (*p == '{') brace_count++;\n        else if (*p == '}') brace_count--;\n        else if (*p == '[') bracket_count++;\n        else if (*p == ']') bracket_count--;\n        \n        // Negative counts indicate malformed JSON\n        if (brace_count < 0 || bracket_count < 0) return 0;\n    }\n    \n    // All braces and brackets must be balanced and not in a string\n    return (brace_count == 0 && bracket_count == 0 && !in_string);\n}\n\n// Fetch JSON from URL with secure HTTPS settings\nstatic char* fetch_json_from_url(const char *url) {\n    CURL *curl = NULL;\n    CURLcode res;\n    ResponseBuffer buffer = {0};\n    char *result = NULL;\n    \n    // Validate URL before any network operation\n    if (!validate_url(url)) {\n        fprintf(stderr, "Error: Invalid or insecure URL. Only HTTPS URLs are allowed.\
");\n        return NULL;\n    }\n    \n    // Initialize response buffer\n    if (!init_response_buffer(&buffer)) {\n        fprintf(stderr, "Error: Memory allocation failed.\
");\n        return NULL;\n    }\n    \n    // Initialize libcurl\n    curl = curl_easy_init();\n    if (!curl) {\n        fprintf(stderr, "Error: Failed to initialize CURL.\
");\n        free_response_buffer(&buffer);\n        return NULL;\n    }\n    \n    // Set URL\n    curl_easy_setopt(curl, CURLOPT_URL, url);\n    \n    // Security: Enable SSL/TLS certificate verification\n    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);\n    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);\n    \n    // Security: Enforce TLS 1.2 minimum\n    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);\n    \n    // Security: Restrict to HTTPS protocol only\n    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);\n    \n    // Set write callback to handle response\n    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);\n    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);\n    \n    // Security: Set reasonable timeout (30 seconds)\n    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);\n    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);\n    \n    // Security: Limit redirects to prevent redirect loops\n    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);\n    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);\n    \n    // Security: Only allow HTTPS for redirects\n    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);\n    \n    // Perform the request\n    res = curl_easy_perform(curl);\n    \n    if (res != CURLE_OK) {\n        fprintf(stderr, "Error: Failed to fetch URL: %s\
", curl_easy_strerror(res));\n        curl_easy_cleanup(curl);\n        free_response_buffer(&buffer);\n        return NULL;\n    }\n    \n    // Check HTTP response code\n    long response_code = 0;\n    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);\n    if (response_code != 200) {\n        fprintf(stderr, "Error: HTTP request failed with code: %ld\
", response_code);\n        curl_easy_cleanup(curl);\n        free_response_buffer(&buffer);\n        return NULL;\n    }\n    \n    curl_easy_cleanup(curl);\n    \n    // Validate JSON structure before returning\n    if (!validate_json_structure(buffer.data)) {\n        fprintf(stderr, "Error: Invalid JSON structure.\
");\n        free_response_buffer(&buffer);\n        return NULL;\n    }\n    \n    // Transfer ownership of data to result\n    result = buffer.data;\n    buffer.data = NULL; // Prevent double free\n    \n    return result;\n}\n\n// Main function with test cases\nint main(int argc, char *argv[]) {\n    char *json_data = NULL;\n    \n    // Test cases when no URL provided\n    if (argc < 2) {\n        printf("Running test cases...\
\
");\n        \n        // Test case 1: Valid HTTPS URL (example - replace with actual test endpoint)\n        printf("Test 1: Valid HTTPS URL\
");\n        json_data = fetch_json_from_url("https://jsonplaceholder.typicode.com/posts/1");\n        if (json_data) {\n            printf("Success: Fetched JSON data\
");\n            printf("Data: %.100s...\
\
", json_data);\n            free(json_data);\n            json_data = NULL;\n        } else {\n            printf("Failed to fetch data\
\
");\n        }\n        \n        // Test case 2: Invalid URL (HTTP instead of HTTPS)\n        printf("Test 2: Invalid URL (HTTP not allowed)\
");\n        json_data = fetch_json_from_url("http://example.com/data.json");\n        if (json_data) {\n            printf("Unexpected success\
\
");\n            free(json_data);\n            json_data = NULL;\n        } else {\n            printf("Correctly rejected HTTP URL\
\
");\n        }\n        \n        // Test case 3: NULL URL\n        printf("Test 3: NULL URL\
");\n        json_data = fetch_json_from_url(NULL);\n        if (json_data) {\n            printf("Unexpected success\
\
");\n            free(json_data);\n            json_data = NULL;\n        } else {\n            printf("Correctly rejected NULL URL\
\
");\n        }\n        \n        // Test case 4: Empty URL\n        printf("Test 4: Empty URL\
");\n        json_data = fetch_json_from_url("");\n        if (json_data) {\n            printf("Unexpected success\
\
");\n            free(json_data);\n            json_data = NULL;\n        } else {\n            printf("Correctly rejected empty URL\
\
");\n        }\n        \n        // Test case 5: Another valid HTTPS URL\n        printf("Test 5: Another valid HTTPS URL\
");\n        json_data = fetch_json_from_url("https://jsonplaceholder.typicode.com/users/1");\n        if (json_data) {\n            printf("Success: Fetched JSON data\
");\n            printf("Data: %.100s...\
\
", json_data);\n            free(json_data);\n            json_data = NULL;\n        } else {\n            printf("Failed to fetch data\
\
");\n        }\n        \n        return 0;\n    }\n    \n    // Validate command line argument length\n    if (strlen(argv[1]) > 2048) {\n        fprintf(stderr, "Error: URL too long.\
");\n        return 1;\n    }\n    \n    // Fetch JSON from provided URL\n    json_data = fetch_json_from_url(argv[1]);\n    \n    if (json_data) {\n        printf("Successfully fetched JSON:\
%s\
", json_data);\n        free(json_data);\n        return 0;\n    } else {\n        fprintf(stderr, "Failed to fetch JSON from URL.\
");
        return 1;
    }
}
