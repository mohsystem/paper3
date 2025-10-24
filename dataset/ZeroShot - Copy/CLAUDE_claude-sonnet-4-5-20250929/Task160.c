
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_URL_LENGTH 2048
#define MAX_HOST_LENGTH 256

int starts_with(const char* str, const char* prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}

void to_lowercase(const char* src, char* dest, size_t max_len) {
    size_t i;
    for (i = 0; i < max_len - 1 && src[i] != '\\0'; i++) {
        dest[i] = tolower(src[i]);
    }
    dest[i] = '\\0';
}

int is_blocked_host(const char* host) {
    char lower_host[MAX_HOST_LENGTH];
    to_lowercase(host, lower_host, MAX_HOST_LENGTH);
    
    if (strcmp(lower_host, "localhost") == 0) return 1;
    if (strcmp(lower_host, "127.0.0.1") == 0) return 1;
    if (strcmp(lower_host, "0.0.0.0") == 0) return 1;
    if (strcmp(lower_host, "::1") == 0) return 1;
    if (strcmp(lower_host, "169.254.169.254") == 0) return 1;
    if (starts_with(lower_host, "192.168.")) return 1;
    if (starts_with(lower_host, "10.")) return 1;
    if (starts_with(lower_host, "172.")) return 1;
    
    return 0;
}

void extract_host(const char* url, char* host, size_t max_len) {
    const char* start = strstr(url, "://");
    if (start == NULL) {
        host[0] = '\\0';
        return;
    }
    start += 3;
    
    const char* end = strchr(start, '/');
    const char* port = strchr(start, ':');
    
    if (port != NULL && (end == NULL || port < end)) {
        end = port;
    }
    
    if (end == NULL) {
        end = start + strlen(start);
    }
    
    size_t len = end - start;
    if (len >= max_len) len = max_len - 1;
    
    strncpy(host, start, len);
    host[len] = '\\0';
}

const char* make_http_request(const char* url_string, char* result, size_t result_size) {
    char lower_url[MAX_URL_LENGTH];
    char host[MAX_HOST_LENGTH];
    
    if (url_string == NULL || strlen(url_string) == 0) {
        snprintf(result, result_size, "Error: Empty URL");
        return result;
    }
    
    to_lowercase(url_string, lower_url, MAX_URL_LENGTH);
    
    if (!starts_with(lower_url, "http://") && !starts_with(lower_url, "https://")) {
        snprintf(result, result_size, "Error: Only HTTP and HTTPS protocols are allowed");
        return result;
    }
    
    extract_host(url_string, host, MAX_HOST_LENGTH);
    
    if (strlen(host) == 0) {
        snprintf(result, result_size, "Error: Invalid URL - no hostname");
        return result;
    }
    
    if (is_blocked_host(host)) {
        snprintf(result, result_size, "Error: Access to internal/private network addresses is blocked");
        return result;
    }
    
    snprintf(result, result_size, 
             "Note: Basic validation passed. Full HTTP implementation requires external libraries.\\n"
             "URL: %s\\nHost: %s\\n"
             "In production, use libcurl or similar library for actual HTTP requests.", 
             url_string, host);
    
    return result;
}

int main() {
    char result[4096];
    
    printf("HTTP Request Maker - Test Cases\\n\\n");
    
    printf("Test 1: Valid HTTPS URL\\n");
    make_http_request("https://www.example.com", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    printf("Test 2: Invalid protocol (file://)\\n");
    make_http_request("file:///etc/passwd", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    printf("Test 3: Blocked host (localhost)\\n");
    make_http_request("http://localhost:8080", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    printf("Test 4: Blocked private IP\\n");
    make_http_request("http://192.168.1.1", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    printf("Test 5: Invalid URL\\n");
    make_http_request("not-a-valid-url", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    return 0;
}
