
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

bool isValidTarget(const char* target) {
    if (target == NULL || strlen(target) == 0) {
        return false;
    }
    
    regex_t regex;
    int result;
    
    result = regcomp(&regex, ".*example\\\\.com.*", REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return false;
    }
    
    result = regexec(&regex, target, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0);
}

const char* handleRedirect(const char* target) {
    if (target != NULL && isValidTarget(target)) {
        return target;
    }
    return "/";
}

int main() {
    // Test cases
    printf("Test 1: %s (Expected: true)\\n", 
           isValidTarget("https://example.com") ? "true" : "false");
    printf("Test 2: %s (Expected: true)\\n", 
           isValidTarget("https://www.example.com/page") ? "true" : "false");
    printf("Test 3: %s (Expected: true)\\n", 
           isValidTarget("https://subdomain.example.com") ? "true" : "false");
    printf("Test 4: %s (Expected: false)\\n", 
           isValidTarget("https://google.com") ? "true" : "false");
    printf("Test 5: %s (Expected: false)\\n", 
           isValidTarget("https://malicious.com") ? "true" : "false");
    
    return 0;
}
