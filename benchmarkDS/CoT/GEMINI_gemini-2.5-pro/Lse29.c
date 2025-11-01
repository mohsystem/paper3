#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Forward declaration
const char* get_redirect_url(const char* target);

// The Lse29 class equivalent structure in C
typedef struct {
    const char* (*getRedirectUrl)(const char* target);
} Lse29;

// Function to initialize the struct with function pointers
Lse29 Lse29_init() {
    Lse29 instance;
    instance.getRedirectUrl = get_redirect_url;
    return instance;
}

// Securely checks if the target URL's host is example.com or a subdomain.
const char* get_redirect_url(const char* target) {
    if (target == NULL || *target == '\0') {
        return "/";
    }

    const char* host_start = strstr(target, "://");
    if (host_start == NULL) {
        return "/"; // Not a valid absolute URL
    }
    host_start += 3; // Move past "://"

    const char* host_end = strpbrk(host_start, "/?#:");
    size_t host_len;
    if (host_end == NULL) {
        host_len = strlen(host_start);
    } else {
        host_len = host_end - host_start;
    }

    if (host_len == 0) {
        return "/";
    }

    // Secure check for host
    // Case 1: host is exactly "example.com"
    const char* main_domain = "example.com";
    size_t main_domain_len = strlen(main_domain);
    if (host_len == main_domain_len && strncasecmp(host_start, main_domain, host_len) == 0) {
        return target;
    }

    // Case 2: host ends with ".example.com"
    const char* suffix = ".example.com";
    size_t suffix_len = strlen(suffix);
    if (host_len > suffix_len && strncasecmp(host_start + host_len - suffix_len, suffix, suffix_len) == 0) {
        return target;
    }

    return "/";
}

void runTest(const char* name, const char* input, const char* expected) {
    Lse29 lse29 = Lse29_init();
    const char* result = lse29.getRedirectUrl(input);
    printf("%s: %s\n", name, strcmp(result, expected) == 0 ? "Pass" : "Fail");
    printf("  Input: %s\n", input ? input : "NULL");
    printf("  Output: %s, Expected: %s\n", result, expected);
}

int main() {
    printf("--- C Test Cases ---\n");
    
    runTest("1. Valid Main Domain", "http://example.com/home", "http://example.com/home");
    runTest("2. Valid Subdomain", "https://sub.example.com/path?q=1", "https://sub.example.com/path?q=1");
    runTest("3. Invalid Domain", "http://evilexample.com", "/");
    runTest("4. Tricky Invalid Domain", "http://evil.com?p=example.com", "/");
    runTest("5. Malformed/Relative URL", "/local/path", "/");

    return 0;
}