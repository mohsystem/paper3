
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

#define DEFAULT_URL "https://default.example.com"
#define MAX_URL_LENGTH 1024

// Global regex pattern compiled once
static regex_t url_pattern;
static int pattern_compiled = 0;

void init_pattern() {
    if (!pattern_compiled) {
        // Compile the regular expression once
        int ret = regcomp(&url_pattern, "^https\\\\?://[a-zA-Z0-9.-]\\\\+\\\\.[a-zA-Z]\\\\{2,\\\\}\\\\(/.*\\\\)\\\\?$", REG_EXTENDED);
        if (ret == 0) {
            pattern_compiled = 1;
        }
    }
}

char* get_redirect_url(const char* target_url, char* result) {
    init_pattern();
    
    if (target_url == NULL || strlen(target_url) == 0) {
        strcpy(result, DEFAULT_URL);
        return result;
    }
    
    // If the target URL matches the pattern, use it as redirect target
    if (pattern_compiled && regexec(&url_pattern, target_url, 0, NULL, 0) == 0) {
        strcpy(result, target_url);
        return result;
    }
    
    // If the target URL does not match, use default URL
    strcpy(result, DEFAULT_URL);
    return result;
}

void cleanup_pattern() {
    if (pattern_compiled) {
        regfree(&url_pattern);
        pattern_compiled = 0;
    }
}

int main() {
    // Test cases
    const char* test_cases[] = {
        "https://www.example.com/page",
        "http://test.com",
        "invalid-url",
        "ftp://wrong-protocol.com",
        "https://secure-site.org/path/to/resource"
    };
    
    char result[MAX_URL_LENGTH];
    
    printf("Testing URL Redirect Logic:\\n");
    for (int i = 0; i < 5; i++) {
        get_redirect_url(test_cases[i], result);
        printf("Input: %s -> Redirect: %s\\n", test_cases[i], result);
    }
    
    cleanup_pattern();
    return 0;
}
