
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_URL_LENGTH 2048
#define MAX_HOST_LENGTH 256

// Structure to hold parsed URL components
typedef struct {
    char scheme[16];
    char host[MAX_HOST_LENGTH];
    bool valid;
} ParsedURL;

// Convert string to lowercase (safe version)
static void str_tolower(char* dest, const char* src, size_t max_len) {
    size_t i = 0;
    if (dest == NULL || src == NULL || max_len == 0) {
        return;
    }
    
    while (src[i] != '\\0' && i < max_len - 1) {
        dest[i] = (char)tolower((unsigned char)src[i]);
        i++;
    }
    dest[i] = '\\0';
}

// Parse URL into components with bounds checking
static ParsedURL parse_url(const char* url) {
    ParsedURL result;
    memset(&result, 0, sizeof(ParsedURL));
    result.valid = false;
    
    if (url == NULL || strlen(url) == 0 || strlen(url) >= MAX_URL_LENGTH) {
        return result;
    }
    
    // Find scheme separator
    const char* scheme_end = strstr(url, "://");
    if (scheme_end == NULL) {
        return result;
    }
    
    // Extract and validate scheme length
    size_t scheme_len = scheme_end - url;
    if (scheme_len == 0 || scheme_len >= sizeof(result.scheme)) {
        return result;
    }
    
    // Copy scheme with bounds check
    strncpy(result.scheme, url, scheme_len);
    result.scheme[scheme_len] = '\\0';
    
    // Convert scheme to lowercase
    for (size_t i = 0; i < scheme_len; i++) {
        result.scheme[i] = (char)tolower((unsigned char)result.scheme[i]);
    }
    
    // Find host
    const char* host_start = scheme_end + 3;
    const char* host_end = strchr(host_start, '/');
    
    size_t host_len;
    if (host_end == NULL) {
        host_len = strlen(host_start);
    } else {
        host_len = host_end - host_start;
    }
    
    // Validate host length
    if (host_len == 0 || host_len >= MAX_HOST_LENGTH) {
        return result;
    }
    
    // Copy host with bounds check
    strncpy(result.host, host_start, host_len);
    result.host[host_len] = '\\0';
    
    // Remove port if present
    char* port_pos = strchr(result.host, ':');
    if (port_pos != NULL) {
        *port_pos = '\\0';
    }
    
    // Convert host to lowercase
    char temp_host[MAX_HOST_LENGTH];
    str_tolower(temp_host, result.host, MAX_HOST_LENGTH);
    strncpy(result.host, temp_host, MAX_HOST_LENGTH - 1);
    result.host[MAX_HOST_LENGTH - 1] = '\\0';
    
    result.valid = (strlen(result.scheme) > 0 && strlen(result.host) > 0);
    return result;
}

// Validates if the URL is from example.com domain
static bool is_valid_example_url(const char* url_string) {
    if (url_string == NULL || strlen(url_string) == 0) {
        return false;
    }
    
    // Parse the URL
    ParsedURL parsed = parse_url(url_string);
    
    if (!parsed.valid) {
        return false;
    }
    
    // Only allow https for security
    if (strcmp(parsed.scheme, "https") != 0) {
        return false;
    }
    
    // Check if host is exactly example.com
    if (strcmp(parsed.host, "example.com") == 0) {
        return true;
    }
    
    // Check if it ends with .example.com (subdomain)
    const char* suffix = ".example.com";
    size_t host_len = strlen(parsed.host);
    size_t suffix_len = strlen(suffix);
    
    if (host_len > suffix_len) {
        const char* host_suffix = parsed.host + (host_len - suffix_len);
        if (strcmp(host_suffix, suffix) == 0) {
            return true;
        }
    }
    
    return false;
}

// Simulated redirect function
static const char* redirect_invalid_url(const char* url) {
    if (is_valid_example_url(url)) {
        return "REDIRECT";
    } else {
        return "ERROR: Invalid URL. Only example.com URLs are allowed.";
    }
}

int main(void) {
    // Test cases
    printf("Test Case 1 - Valid example.com URL:\\n");
    const char* test1 = "https://example.com/page";
    printf("URL: %s\\n", test1);
    printf("Result: %s\\n\\n", redirect_invalid_url(test1));
    
    printf("Test Case 2 - Valid subdomain:\\n");
    const char* test2 = "https://sub.example.com/path";
    printf("URL: %s\\n", test2);
    printf("Result: %s\\n\\n", redirect_invalid_url(test2));
    
    printf("Test Case 3 - Invalid domain:\\n");
    const char* test3 = "https://evil.com";
    printf("URL: %s\\n", test3);
    printf("Result: %s\\n\\n", redirect_invalid_url(test3));
    
    printf("Test Case 4 - Bypass attempt:\\n");
    const char* test4 = "https://example.com.evil.com";
    printf("URL: %s\\n", test4);
    printf("Result: %s\\n\\n", redirect_invalid_url(test4));
    
    printf("Test Case 5 - HTTP not allowed:\\n");
    const char* test5 = "http://example.com";
    printf("URL: %s\\n", test5);
    printf("Result: %s\\n", redirect_invalid_url(test5));
    
    return 0;
}
