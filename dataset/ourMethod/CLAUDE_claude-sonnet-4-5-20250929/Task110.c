
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Maximum URL length to prevent buffer overflow and DoS attacks */
#define MAX_URL_LENGTH 2048
#define MAX_DOMAIN_LENGTH 253
#define MAX_PORT_LENGTH 5

/* Structure to hold validation result */
typedef struct {
    bool is_valid;
    char error_message[256];
} ValidationResult;

/* Initialize validation result with safe defaults */
static void init_validation_result(ValidationResult* result) {
    if (result == NULL) {
        return;
    }
    result->is_valid = false;
    memset(result->error_message, 0, sizeof(result->error_message));
}

/* Safely copy string with bounds checking */
static bool safe_strcpy(char* dest, size_t dest_size, const char* src) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return false;
    }
    
    size_t src_len = strlen(src);
    if (src_len >= dest_size) {
        return false;
    }
    
    /* Use snprintf for safe copy with null termination */
    snprintf(dest, dest_size, "%s", src);
    return true;
}

/* Check for null bytes in string - security measure against injection */
static bool contains_null_byte(const char* str, size_t len) {
    if (str == NULL) {
        return true;
    }
    
    /* Check for embedded null bytes before the actual string end */
    size_t actual_len = strnlen(str, len);
    return actual_len < len - 1 && actual_len != strlen(str);
}

/* Validate URL length with bounds checking */
static bool validate_length(const char* url, ValidationResult* result) {
    if (url == NULL) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "URL is NULL");
        }
        return false;
    }
    
    size_t len = strnlen(url, MAX_URL_LENGTH + 1);
    
    if (len == 0) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "URL is empty");
        }
        return false;
    }
    
    if (len > MAX_URL_LENGTH) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "URL exceeds maximum length");
        }
        return false;
    }
    
    /* Security: check for null byte injection */
    if (contains_null_byte(url, len)) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "URL contains null byte");
        }
        return false;
    }
    
    return true;
}

/* Validate scheme is http or https only - security restriction */
static bool validate_scheme(const char* url, size_t* scheme_end, 
                           ValidationResult* result) {
    if (url == NULL || scheme_end == NULL) {
        return false;
    }
    
    /* Check for https:// */
    if (strncmp(url, "https://", 8) == 0) {
        *scheme_end = 8;
        return true;
    }
    
    /* Check for http:// */
    if (strncmp(url, "http://", 7) == 0) {
        *scheme_end = 7;
        return true;
    }
    
    if (result != NULL) {
        safe_strcpy(result->error_message, sizeof(result->error_message), 
                   "Invalid scheme - only http and https allowed");
    }
    return false;
}

/* Validate domain name characters to prevent injection attacks */
static bool validate_domain_chars(const char* domain, size_t len) {
    if (domain == NULL || len == 0) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)domain[i];
        /* Only allow alphanumeric, dot, hyphen, and colon for port */
        if (!isalnum(c) && c != '.' && c != '-' && c != ':') {
            return false;
        }
    }
    return true;
}

/* Extract and validate domain portion */
static bool validate_domain(const char* url, size_t scheme_end, 
                           ValidationResult* result) {
    if (url == NULL) {
        return false;
    }
    
    size_t url_len = strlen(url);
    
    /* Bounds check */
    if (scheme_end >= url_len) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "Invalid URL structure");
        }
        return false;
    }
    
    /* Find domain end (first slash after scheme or end of string) */
    const char* domain_start = url + scheme_end;
    const char* domain_end = strchr(domain_start, '/');
    size_t domain_len;
    
    if (domain_end == NULL) {
        domain_len = strlen(domain_start);
    } else {
        domain_len = (size_t)(domain_end - domain_start);
    }
    
    /* Validate domain length */
    if (domain_len == 0 || domain_len > MAX_DOMAIN_LENGTH) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "Invalid domain length");
        }
        return false;
    }
    
    /* Validate domain characters - security check */
    if (!validate_domain_chars(domain_start, domain_len)) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "Invalid characters in domain");
        }
        return false;
    }
    
    /* Check for minimum valid domain structure (at least one dot) */
    if (strchr(domain_start, '.') == NULL && strchr(domain_start, ':') == NULL) {
        if (result != NULL) {
            safe_strcpy(result->error_message, sizeof(result->error_message), 
                       "Invalid domain structure");
        }
        return false;
    }
    
    return true;
}

/* Main validation function with comprehensive security checks */
bool validate_url(const char* url, ValidationResult* result) {
    /* Initialize result */
    if (result != NULL) {
        init_validation_result(result);
    }
    
    /* Validate input length and check for security issues */
    if (!validate_length(url, result)) {
        return false;
    }
    
    /* Validate scheme (only http/https for security) */
    size_t scheme_end = 0;
    if (!validate_scheme(url, &scheme_end, result)) {
        return false;
    }
    
    /* Validate domain portion */
    if (!validate_domain(url, scheme_end, result)) {
        return false;
    }
    
    /* If all checks passed, URL is valid */
    if (result != NULL) {
        result->is_valid = true;
        safe_strcpy(result->error_message, sizeof(result->error_message), 
                   "Valid URL");
    }
    
    return true;
}

int main(void) {
    /* Test cases array */
    const char* test_urls[] = {
        "https://www.example.com",
        "http://example.com/path/to/resource",
        "ftp://invalid.com",
        "https://sub.domain.example.com:8080/path?query=value",
        ""
    };
    
    size_t num_tests = sizeof(test_urls) / sizeof(test_urls[0]);
    
    printf("URL Validation Results:\\n");
    printf("=======================\\n\\n");
    
    for (size_t i = 0; i < num_tests; i++) {
        ValidationResult result;
        init_validation_result(&result);
        
        bool is_valid = validate_url(test_urls[i], &result);
        
        /* Safely display URL with bounds checking */
        const char* display_url = (test_urls[i] != NULL && strlen(test_urls[i]) > 0) 
                                  ? test_urls[i] : "(empty string)";
        
        printf("URL: %.80s\\n", display_url);
        printf("Valid: %s\\n", is_valid ? "YES" : "NO");
        
        if (!is_valid && result.error_message[0] != '\\0') {
            printf("Error: %.80s\\n", result.error_message);
        }
        
        printf("\\n");
    }
    
    return 0;
}
