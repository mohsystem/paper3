
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Maximum URL length to prevent excessive memory usage */
#define MAX_URL_LENGTH 2048

/* Convert string to lowercase in place - bounds safe */
static void to_lowercase(char* str, size_t len) {
    if (str == NULL) {
        return;
    }
    for (size_t i = 0; i < len && str[i] != '\\0'; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

/* 
 * Validates that the URL belongs to example.com domain.
 * Returns true only if the URL is valid and from example.com.
 * Prevents open redirect vulnerabilities.
 */
bool is_valid_example_com_url(const char* url_string) {
    if (url_string == NULL) {
        return false;
    }
    
    size_t url_len = strnlen(url_string, MAX_URL_LENGTH + 1);
    
    /* Check length bounds */
    if (url_len == 0 || url_len > MAX_URL_LENGTH) {
        return false;
    }
    
    /* Check for valid scheme (http:// or https://) */
    const char* scheme_end = strstr(url_string, "://");
    if (scheme_end == NULL) {
        return false;
    }
    
    size_t scheme_len = scheme_end - url_string;
    if ((scheme_len != 4 && scheme_len != 5) ||
        (strncmp(url_string, "http://", 7) != 0 && 
         strncmp(url_string, "https://", 8) != 0)) {
        return false;
    }
    
    /* Extract host part */
    const char* host_start = scheme_end + 3;
    const char* path_start = strchr(host_start, '/');
    
    size_t host_len;
    if (path_start != NULL) {
        host_len = path_start - host_start;
    } else {
        host_len = strlen(host_start);
    }
    
    /* Allocate buffer for host with size validation */
    if (host_len == 0 || host_len > 255) { /* Max domain length is 255 */
        return false;
    }
    
    char* host = (char*)calloc(host_len + 1, sizeof(char));
    if (host == NULL) {
        return false;
    }
    
    /* Copy host with bounds checking */
    memcpy(host, host_start, host_len);
    host[host_len] = '\\0';
    
    /* Check for @ symbol (prevents user info injection) */
    if (strchr(host, '@') != NULL) {
        free(host);
        return false;
    }
    
    /* Remove port if present */
    char* port_pos = strchr(host, ':');
    if (port_pos != NULL) {
        *port_pos = '\\0';
        host_len = port_pos - host;
    }
    
    /* Convert to lowercase for comparison */
    to_lowercase(host, host_len);
    
    /* Validate domain is example.com or subdomain */
    bool is_valid = false;
    if (strcmp(host, "example.com") == 0) {
        is_valid = true;
    } else if (host_len > 12) { /* ".example.com" is 12 chars */
        const char* suffix = host + host_len - 12;
        if (strcmp(suffix, ".example.com") == 0) {
            is_valid = true;
        }
    }
    
    /* Clear and free host buffer */
    memset(host, 0, host_len + 1);
    free(host);
    
    return is_valid;
}

/*
 * Simulated redirect function.
 * In a real web application, this would set HTTP redirect headers.
 */
const char* redirect_invalid_url(const char* url) {
    /* Validate input is not NULL */
    if (url == NULL || url[0] == '\\0') {
        return "Error: URL parameter is required";
    }
    
    /* Validate the URL is from example.com domain */
    if (!is_valid_example_com_url(url)) {
        return "Error: Invalid URL. Only example.com URLs are allowed.";
    }
    
    /* In a real web app, this would set Location header and return 302 */
    return "Redirect successful";
}

int main(void) {
    /* Test cases */
    printf("Test Case 1 - Valid example.com URL:\\n");
    const char* test1 = "https://example.com/path";
    printf("%s\\n", is_valid_example_com_url(test1) ? "PASS" : "FAIL");
    printf("%s\\n\\n", redirect_invalid_url(test1));
    
    printf("Test Case 2 - Valid subdomain:\\n");
    const char* test2 = "https://sub.example.com/page";
    printf("%s\\n", is_valid_example_com_url(test2) ? "PASS" : "FAIL");
    printf("%s\\n\\n", redirect_invalid_url(test2));
    
    printf("Test Case 3 - Invalid domain:\\n");
    const char* test3 = "https://evil.com";
    printf("%s\\n", !is_valid_example_com_url(test3) ? "PASS" : "FAIL");
    printf("%s\\n\\n", redirect_invalid_url(test3));
    
    printf("Test Case 4 - Domain confusion attack:\\n");
    const char* test4 = "https://example.com.evil.com";
    printf("%s\\n", !is_valid_example_com_url(test4) ? "PASS" : "FAIL");
    printf("%s\\n\\n", redirect_invalid_url(test4));
    
    printf("Test Case 5 - Invalid scheme:\\n");
    const char* test5 = "javascript:alert(1)";
    printf("%s\\n", !is_valid_example_com_url(test5) ? "PASS" : "FAIL");
    printf("%s\\n\\n", redirect_invalid_url(test5));
    
    return 0;
}
