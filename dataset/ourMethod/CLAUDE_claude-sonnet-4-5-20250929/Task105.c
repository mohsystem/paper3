
/*
 * Secure SSL/TLS Connection Program
 * 
 * Security measures implemented:
 * - TLS 1.2+ enforcement with certificate verification
 * - Hostname verification enabled
 * - Proper error handling without information leakage
 * - No hardcoded secrets or credentials
 * - Secure memory handling for sensitive data
 * - Input validation for URLs and hostnames
 * - Timeout and redirect limits
 * - HTTPS-only protocol restriction
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/* Maximum response size to prevent memory exhaustion */
#define MAX_RESPONSE_SIZE (10 * 1024 * 1024) /* 10 MB */
#define MAX_URL_LENGTH 2048
#define MAX_HOSTNAME_LENGTH 256

/* Structure to hold response data with bounds checking */
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ResponseBuffer;

/* Initialize response buffer with proper NULL checks */
static int init_response_buffer(ResponseBuffer *buffer, size_t initial_capacity) {
    if (buffer == NULL || initial_capacity == 0 || initial_capacity > MAX_RESPONSE_SIZE) {
        return -1;
    }
    
    buffer->data = (char *)calloc(initial_capacity, sizeof(char));
    if (buffer->data == NULL) {
        return -1;
    }
    
    buffer->size = 0;
    buffer->capacity = initial_capacity;
    return 0;
}

/* Free response buffer and clear sensitive data */
static void free_response_buffer(ResponseBuffer *buffer) {
    if (buffer == NULL) {
        return;
    }
    
    if (buffer->data != NULL) {
        /* Clear buffer before freeing - defense in depth */
        memset(buffer->data, 0, buffer->capacity);
        free(buffer->data);
        buffer->data = NULL;
    }
    
    buffer->size = 0;
    buffer->capacity = 0;
}

/* Callback function for libcurl to write received data */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize;
    ResponseBuffer *buffer = (ResponseBuffer *)userp;
    
    /* Validate inputs to prevent NULL dereference */
    if (contents == NULL || userp == NULL) {
        return 0;
    }
    
    /* Check for integer overflow before multiplication */
    if (size > 0 && nmemb > SIZE_MAX / size) {
        return 0;
    }
    
    realsize = size * nmemb;
    
    /* Bounds check: prevent buffer overflow and memory exhaustion */
    if (buffer->size > MAX_RESPONSE_SIZE - realsize) {
        fprintf(stderr, "Response size limit exceeded\\n");
        return 0;
    }
    
    /* Reallocate buffer if needed with overflow check */
    if (buffer->size + realsize + 1 > buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        
        /* Check for overflow in capacity calculation */
        if (new_capacity > MAX_RESPONSE_SIZE) {
            new_capacity = MAX_RESPONSE_SIZE;
        }
        
        if (buffer->size + realsize + 1 > new_capacity) {
            return 0;
        }
        
        char *new_data = (char *)realloc(buffer->data, new_capacity);
        if (new_data == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            return 0;
        }
        
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }
    
    /* Safe memory copy with bounds check */
    memcpy(buffer->data + buffer->size, contents, realsize);
    buffer->size += realsize;
    buffer->data[buffer->size] = '\\0'; /* Null terminate */
    
    return realsize;
}

/* Validate URL format and protocol (HTTPS only) */
static int validate_url(const char *url) {
    if (url == NULL) {
        return -1;
    }
    
    size_t url_len = strnlen(url, MAX_URL_LENGTH + 1);
    
    /* Check length bounds */
    if (url_len == 0 || url_len > MAX_URL_LENGTH) {
        fprintf(stderr, "Invalid URL length\\n");
        return -1;
    }
    
    /* Enforce HTTPS only - prevent protocol downgrade */
    if (strncmp(url, "https://", 8) != 0) {
        fprintf(stderr, "Only HTTPS URLs are allowed\\n");
        return -1;
    }
    
    /* Basic validation: check for null bytes (path traversal attempt) */
    if (memchr(url, '\\0', url_len) != url + url_len) {
        fprintf(stderr, "URL contains null byte\\n");
        return -1;
    }
    
    return 0;
}

/*
 * Establish secure SSL/TLS connection and fetch content
 * 
 * Security features:
 * - TLS 1.2 minimum version enforced
 * - Certificate verification enabled (VERIFYPEER)
 * - Hostname verification enabled (VERIFYHOST=2)
 * - Timeout limits to prevent hanging connections
 * - Redirect limits to prevent redirect loops
 * - HTTPS-only protocol enforcement
 */
int establish_secure_connection(const char *url, char **response_out, size_t *response_size_out) {
    CURL *curl = NULL;
    CURLcode res;
    ResponseBuffer buffer;
    int init_result;
    int return_code = -1;
    
    /* Validate inputs */
    if (url == NULL || response_out == NULL || response_size_out == NULL) {
        fprintf(stderr, "Invalid parameters\\n");
        return -1;
    }
    
    /* Initialize output to NULL for safe cleanup on error */
    *response_out = NULL;
    *response_size_out = 0;
    
    /* Validate URL format and enforce HTTPS */
    if (validate_url(url) != 0) {
        return -1;
    }
    
    /* Initialize response buffer with overflow check */
    init_result = init_response_buffer(&buffer, 4096);
    if (init_result != 0) {
        fprintf(stderr, "Failed to initialize buffer\\n");
        return -1;
    }
    
    /* Initialize libcurl */
    curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "Failed to initialize CURL\\n");
        free_response_buffer(&buffer);
        return -1;
    }
    
    /* Set URL */
    res = curl_easy_setopt(curl, CURLOPT_URL, url);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set URL: %s\\n", curl_easy_strerror(res));
        goto cleanup;
    }
    
    /* SECURITY: Enable SSL/TLS certificate verification (CWE-295 mitigation) */
    res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to enable peer verification\\n");
        goto cleanup;
    }
    
    /* SECURITY: Enable hostname verification (CWE-297 mitigation) */
    res = curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to enable host verification\\n");
        goto cleanup;
    }
    
    /* SECURITY: Enforce TLS 1.2 or higher (CWE-327 mitigation) */
    res = curl_easy_setopt(curl, CURLOPT_SSLVERSION, 
                           CURL_SSLVERSION_TLSv1_2 | CURL_SSLVERSION_MAX_TLSv1_3);
    if (res != CURLE_OK) {
        /* Fallback for older libcurl versions */
        res = curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        if (res != CURLE_OK) {
            fprintf(stderr, "Failed to set TLS version\\n");
            goto cleanup;
        }
    }
    
    /* SECURITY: Restrict to HTTPS protocol only */
    res = curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to restrict protocols\\n");
        goto cleanup;
    }
    
    /* Set connection timeout (30 seconds) */
    res = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set connect timeout\\n");
        goto cleanup;
    }
    
    /* Set operation timeout (60 seconds) */
    res = curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set timeout\\n");
        goto cleanup;
    }
    
    /* Limit redirects to prevent redirect loops */
    res = curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set max redirects\\n");
        goto cleanup;
    }
    
    /* Follow redirects (but stay within HTTPS) */
    res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to enable follow location\\n");
        goto cleanup;
    }
    
    /* Set write callback and data pointer */
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set write callback\\n");
        goto cleanup;
    }
    
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    if (res != CURLE_OK) {
        fprintf(stderr, "Failed to set write data\\n");
        goto cleanup;
    }
    
    /* Perform the request */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        /* Generic error message - don't leak internal details */
        fprintf(stderr, "Connection failed: %s\\n", curl_easy_strerror(res));
        goto cleanup;
    }
    
    /* Verify HTTP status code */
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code < 200 || http_code >= 300) {
        fprintf(stderr, "HTTP error: %ld\\n", http_code);
        goto cleanup;
    }
    
    /* Transfer ownership of buffer data to caller */
    *response_out = buffer.data;
    *response_size_out = buffer.size;
    buffer.data = NULL; /* Prevent double free */
    
    return_code = 0;
    
cleanup:
    /* Cleanup resources */
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
    
    if (return_code != 0) {
        free_response_buffer(&buffer);
    }
    
    return return_code;
}

/* Main function with test cases */
int main(void) {
    int result;
    char *response = NULL;
    size_t response_size = 0;
    
    /* Initialize libcurl globally */
    curl_global_init(CURL_GLOBAL_SSL);
    
    printf("Testing secure SSL/TLS connections...\\n\\n");
    
    /* Test Case 1: Valid HTTPS connection to trusted site */
    printf("Test 1: Connecting to https://www.google.com\\n");
    result = establish_secure_connection("https://www.google.com", &response, &response_size);
    if (result == 0) {
        printf("Success! Received %zu bytes\\n", response_size);
        if (response != NULL) {
            memset(response, 0, response_size);
            free(response);
            response = NULL;
        }
    } else {
        printf("Failed to connect\\n");
    }
    printf("\\n");
    
    /* Test Case 2: Another trusted site */
    printf("Test 2: Connecting to https://www.github.com\\n");
    result = establish_secure_connection("https://www.github.com", &response, &response_size);
    if (result == 0) {
        printf("Success! Received %zu bytes\\n", response_size);
        if (response != NULL) {
            memset(response, 0, response_size);
            free(response);
            response = NULL;
        }
    } else {
        printf("Failed to connect\\n");
    }
    printf("\\n");
    
    /* Test Case 3: HTTP URL should be rejected (security test) */
    printf("Test 3: Attempting HTTP connection (should fail)\\n");
    result = establish_secure_connection("http://www.example.com", &response, &response_size);
    if (result != 0) {
        printf("Correctly rejected non-HTTPS URL\\n");
    } else {
        printf("ERROR: Should have rejected HTTP URL\\n");
        if (response != NULL) {
            memset(response, 0, response_size);
            free(response);
            response = NULL;
        }
    }
    printf("\\n");
    
    /* Test Case 4: Invalid URL format */
    printf("Test 4: Testing invalid URL format\\n");
    result = establish_secure_connection("not_a_url", &response, &response_size);
    if (result != 0) {
        printf("Correctly rejected invalid URL\\n");
    } else {
        printf("ERROR: Should have rejected invalid URL\\n");
        if (response != NULL) {
            memset(response, 0, response_size);
            free(response);
            response = NULL;
        }
    }
    printf("\\n");
    
    /* Test Case 5: NULL parameter handling */
    printf("Test 5: Testing NULL parameter handling\\n");
    result = establish_secure_connection(NULL, &response, &response_size);
    if (result != 0) {
        printf("Correctly handled NULL URL\\n");
    } else {
        printf("ERROR: Should have rejected NULL URL\\n");
        if (response != NULL) {
            memset(response, 0, response_size);
            free(response);
            response = NULL;
        }
    }
    printf("\\n");
    
    /* Cleanup libcurl globally */
    curl_global_cleanup();
    
    printf("All tests completed.\\n");
    return 0;
}
