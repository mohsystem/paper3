
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

char* processRedirect(const char* target) {
    regex_t regex;
    int reti;
    char* result;
    
    // Regular expression: one or more alphanumeric characters or hyphens followed by a period
    const char* pattern = "^[a-zA-Z0-9-]+\\\\..*";
    
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\\n");
        return strdup("/");
    }
    
    reti = regexec(&regex, target, 0, NULL, 0);
    regfree(&regex);
    
    if (!reti) {
        result = strdup(target);
    } else {
        result = strdup("/");
    }
    
    return result;
}

int main() {
    // Test cases
    const char* testCases[] = {
        "example.com",
        "test-site.org",
        "my-page.html",
        ".invalid",
        "noperiod"
    };
    
    printf("C Test Results:\\n");
    for (int i = 0; i < 5; i++) {
        char* result = processRedirect(testCases[i]);
        printf("Input: '%s' -> Redirect to: '%s'\\n", testCases[i], result);
        free(result);
    }
    
    return 0;
}
