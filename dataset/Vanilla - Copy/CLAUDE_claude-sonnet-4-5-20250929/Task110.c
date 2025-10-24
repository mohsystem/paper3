
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

bool validateURL(const char* url) {
    if (url == NULL || strlen(url) == 0) {
        return false;
    }
    
    // Regular expression pattern for URL validation
    const char* urlPattern = "^(https?|ftp)://"
                            "("
                            "([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,}"
                            "|"
                            "localhost"
                            "|"
                            "((\\\\d{1,3}\\\\.){3}\\\\d{1,3})"
                            ")"
                            "(:[0-9]+)?"
                            "(/[a-zA-Z0-9._~:/?#\\\\[\\\\]@!$&'()*+,;=-]*)?$";
    
    regex_t regex;
    int result;
    
    // Compile regular expression
    result = regcomp(&regex, urlPattern, REG_EXTENDED);
    if (result != 0) {
        return false;
    }
    
    // Execute regular expression
    result = regexec(&regex, url, 0, NULL, 0);
    
    // Free memory allocated for regex
    regfree(&regex);
    
    return (result == 0);
}

int main() {
    // Test cases
    const char* testUrls[] = {
        "https://www.example.com",
        "http://localhost:8080/path/to/resource",
        "ftp://192.168.1.1/files",
        "invalid-url-without-protocol",
        "https://subdomain.example.co.uk:3000/api?param=value"
    };
    
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);
    
    printf("URL Validation Results:\\n");
    for (int i = 0; i < numTests; i++) {
        bool isValid = validateURL(testUrls[i]);
        printf("%s -> %s\\n", testUrls[i], isValid ? "Valid" : "Invalid");
    }
    
    return 0;
}
