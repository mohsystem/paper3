
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

int isValidTarget(const char* target) {
    if (target == NULL || strlen(target) == 0) {
        return 0;
    }
    
    regex_t regex;
    int reti;
    
    // Compile regular expression
    reti = regcomp(&regex, "^https?://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$", REG_EXTENDED);
    if (reti) {
        return 0;
    }
    
    // Execute regular expression
    reti = regexec(&regex, target, 0, NULL, 0);
    regfree(&regex);
    
    return (reti == 0) ? 1 : 0;
}

char* getRedirectUrl(const char* target) {
    static char result[512];
    
    if (isValidTarget(target)) {
        strncpy(result, target, sizeof(result) - 1);
        result[sizeof(result) - 1] = '\\0';
    } else {
        strcpy(result, "/");
    }
    
    return result;
}

int main() {
    // Test cases
    const char* testCases[] = {
        "http://example.com",
        "https://subdomain.example.com/path",
        "http://malicious.com",
        "https://example.com.malicious.com",
        NULL
    };
    
    printf("Test Results:\\n");
    for (int i = 0; i < 5; i++) {
        char* result = getRedirectUrl(testCases[i]);
        printf("Test Case %d: Input: \\"%s\\" -> Output: \\"%s\\"\\n", 
               i + 1, 
               testCases[i] ? testCases[i] : "NULL", 
               result);
    }
    
    return 0;
}
