
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_URL_LENGTH 2048
#define MAX_DOMAIN_LENGTH 256

/**
 * Converts string to lowercase in place - bounds safe
 */
void toLowerBounded(char* str, size_t maxLen) {
    if (str == NULL) return;
    
    size_t len = strnlen(str, maxLen);
    for (size_t i = 0; i < len; i++) {
        str[i] = (char)tolower((unsigned char)str[i]);
    }
}

/**
 * Extracts domain from URL with bounds checking
 * Returns true if successfully extracted, false otherwise
 */
bool extractDomain(const char* url, char* domain, size_t domainSize, char* scheme, size_t schemeSize) {
    // Initialize outputs to empty - fail safe
    if (domain != NULL && domainSize > 0) {
        domain[0] = '\\0';
    }
    if (scheme != NULL && schemeSize > 0) {
        scheme[0] = '\\0';
    }
    
    // Input validation
    if (url == NULL || domain == NULL || scheme == NULL) {
        return false;
    }
    
    // Find scheme separator with bounds check
    const char* schemeSep = strstr(url, "://");
    if (schemeSep == NULL) {
        return false;
    }
    
    // Extract scheme with bounds check
    size_t schemeLen = schemeSep - url;
    if (schemeLen >= schemeSize) {
        return false;
    }
    memcpy(scheme, url, schemeLen);
    scheme[schemeLen] = '\\0';
    
    // Extract domain starting after ://
    const char* hostStart = schemeSep + 3;
    const char* hostEnd = hostStart;
    
    // Find end of host (first /, ?, or # or end of string)
    while (*hostEnd != '\\0' && *hostEnd != '/' && *hostEnd != '?' && *hostEnd != '#') {
        hostEnd++;
    }
    
    // Bounds check for domain buffer
    size_t hostLen = hostEnd - hostStart;
    if (hostLen >= domainSize) {
        return false;
    }
    
    // Copy domain with null termination
    memcpy(domain, hostStart, hostLen);
    domain[hostLen] = '\\0';
    
    return true;
}

/**
 * Redirects to the URL if domain is example.com, otherwise returns 404.
 * 
 * @param url The URL to validate and redirect
 * @param result Buffer to store result message
 * @param resultSize Size of result buffer
 */
void redirectUrl(const char* url, char* result, size_t resultSize) {
    const char* notFound = "404 - Not Found";
    char domain[MAX_DOMAIN_LENGTH];
    char scheme[64];
    
    // Initialize result buffer - fail safe
    if (result == NULL || resultSize == 0) {
        return;
    }
    result[0] = '\\0';
    
    // Input validation: reject NULL or empty URLs
    if (url == NULL || url[0] == '\\0') {
        snprintf(result, resultSize, "%s", notFound);
        return;
    }
    
    // Limit URL length to prevent resource exhaustion
    size_t urlLen = strnlen(url, MAX_URL_LENGTH + 1);
    if (urlLen > MAX_URL_LENGTH) {
        snprintf(result, resultSize, "%s", notFound);
        return;
    }
    
    // Extract domain and scheme safely with bounds checking
    if (!extractDomain(url, domain, sizeof(domain), scheme, sizeof(scheme))) {
        snprintf(result, resultSize, "%s", notFound);
        return;
    }
    
    // Validate domain is present - fail closed
    if (domain[0] == '\\0') {
        snprintf(result, resultSize, "%s", notFound);
        return;
    }
    
    // Normalize to lowercase - prevent bypass
    toLowerBounded(domain, sizeof(domain));
    toLowerBounded(scheme, sizeof(scheme));
    
    // Whitelist exact match only - prevent subdomain attacks
    if (strcmp(domain, "example.com") == 0) {
        // Only redirect if scheme is http or https - prevent javascript: data: etc
        if (strcmp(scheme, "http") == 0 || strcmp(scheme, "https") == 0) {
            snprintf(result, resultSize, "Redirect to: %s", url);
            return;
        }
    }
    
    // Fail closed - reject anything not explicitly allowed
    snprintf(result, resultSize, "%s", notFound);
}

int main(void) {
    char result[MAX_URL_LENGTH + 64];
    
    // Test cases
    const char* testUrls[] = {
        "http://example.com/path",
        "https://example.com/secure",
        "http://notexample.com/path",
        "http://subdomain.example.com/path",
        "javascript:alert(1)"
    };
    
    size_t numTests = sizeof(testUrls) / sizeof(testUrls[0]);
    
    for (size_t i = 0; i < numTests; i++) {
        printf("URL: %s\\n", testUrls[i]);
        redirectUrl(testUrls[i], result, sizeof(result));
        printf("Result: %s\\n\\n", result);
    }
    
    return 0;
}
