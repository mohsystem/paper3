#include <stdio.h>
#include <string.h>

/**
 * Parses a URL to check if its domain is "example.com".
 *
 * @param url The URL to check.
 * @return "redirected" if the domain is "example.com", otherwise "404 error".
 */
const char* redirect_url(const char* url) {
    // Find the protocol separator "://"
    const char* protocol_sep = strstr(url, "://");
    if (protocol_sep == NULL) {
        return "404 error"; // No protocol found
    }

    // The domain starts right after the separator
    const char* domain_start = protocol_sep + 3;
    
    // Find the length of the domain by looking for the first '/' or ':'
    size_t domain_len = strcspn(domain_start, "/:");

    const char* target_domain = "example.com";
    size_t target_len = strlen(target_domain);

    // Compare the extracted domain with "example.com"
    if (domain_len == target_len && strncmp(domain_start, target_domain, target_len) == 0) {
        return "redirected";
    } else {
        return "404 error";
    }
}

int main() {
    const char* test_urls[] = {
        "https://example.com/home",
        "http://www.example.com/test",
        "ftp://google.com/files",
        "https://example.com:8080/path?a=1",
        "not_a_valid_url"
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("URL: %s -> %s\n", test_urls[i], redirect_url(test_urls[i]));
    }

    return 0;
}