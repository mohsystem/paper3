
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct {
    char status[20];
    char message[256];
    char url[512];
} Response;

void to_lowercase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

char* extract_host(const char* url, char* host, size_t host_size) {
    const char* start = url;
    
    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
    } else if (strncmp(url, "https://", 8) == 0) {
        start = url + 8;
    }
    
    const char* end = strchr(start, '/');
    if (end == NULL) {
        end = start + strlen(start);
    }
    
    size_t len = end - start;
    if (len >= host_size) {
        len = host_size - 1;
    }
    
    strncpy(host, start, len);
    host[len] = '\\0';
    
    char* colon = strchr(host, ':');
    if (colon != NULL) {
        *colon = '\\0';
    }
    
    return host;
}

Response redirect_invalid_url(const char* url_string) {
    Response response;
    memset(&response, 0, sizeof(Response));
    
    if (url_string == NULL || strlen(url_string) == 0) {
        strcpy(response.status, "error");
        strcpy(response.message, "URL parameter is required");
        return response;
    }
    
    char host[256];
    extract_host(url_string, host, sizeof(host));
    
    if (strlen(host) == 0) {
        strcpy(response.status, "error");
        strcpy(response.message, "Invalid URL format");
        return response;
    }
    
    to_lowercase(host);
    
    int valid = 0;
    if (strcmp(host, "example.com") == 0) {
        valid = 1;
    } else {
        size_t host_len = strlen(host);
        if (host_len > 11 && strcmp(host + host_len - 11, ".example.com") == 0) {
            valid = 1;
        }
    }
    
    if (valid) {
        strcpy(response.status, "redirect");
        strncpy(response.url, url_string, sizeof(response.url) - 1);
        snprintf(response.message, sizeof(response.message), 
                "Redirecting to: %s", url_string);
    } else {
        strcpy(response.status, "error");
        strcpy(response.message, "Invalid domain. Only example.com domain is allowed");
    }
    
    return response;
}

void print_response(const char* test_name, Response response) {
    printf("%s: {status: %s, message: %s", test_name, response.status, response.message);
    if (strlen(response.url) > 0) {
        printf(", url: %s", response.url);
    }
    printf("}\\n");
}

int main() {
    // Test case 1: Valid example.com URL
    print_response("Test 1", redirect_invalid_url("https://example.com/page"));
    
    // Test case 2: Valid subdomain of example.com
    print_response("Test 2", redirect_invalid_url("https://sub.example.com/path"));
    
    // Test case 3: Invalid domain
    print_response("Test 3", redirect_invalid_url("https://malicious.com/page"));
    
    // Test case 4: Invalid URL format
    print_response("Test 4", redirect_invalid_url("not-a-valid-url"));
    
    // Test case 5: Empty/null URL
    print_response("Test 5", redirect_invalid_url(NULL));
    
    return 0;
}
