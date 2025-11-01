
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

#define PATTERN "^https?://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$"

int isValidTarget(const char* target) {
    if (target == NULL || strlen(target) == 0) {
        return 0;
    }
    
    regex_t regex;
    int result;
    
    result = regcomp(&regex, PATTERN, REG_EXTENDED);
    if (result != 0) {
        return 0;
    }
    
    result = regexec(&regex, target, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0) ? 1 : 0;
}

const char* getRedirectTarget(const char* target) {
    if (isValidTarget(target)) {
        return target;
    }
    return "/";
}

void runTests() {
    printf("Test Cases:\\n");
    printf("1. Valid: http://example.com -> %d\\n", 
           isValidTarget("http://example.com"));
    printf("2. Valid: https://www.example.com/path -> %d\\n", 
           isValidTarget("https://www.example.com/path"));
    printf("3. Invalid: http://malicious.com -> %d\\n", 
           isValidTarget("http://malicious.com"));
    printf("4. Invalid: javascript:alert(1) -> %d\\n", 
           isValidTarget("javascript:alert(1)"));
    printf("5. Valid: http://subdomain.example.com -> %d\\n", 
           isValidTarget("http://subdomain.example.com"));
}

int main() {
    runTests();
    return 0;
}
