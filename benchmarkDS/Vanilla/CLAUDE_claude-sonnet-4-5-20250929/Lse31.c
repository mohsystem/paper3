
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

char* redirect_url(const char* url) {
    static char result[256];
    regex_t regex;
    regmatch_t matches[3];
    
    const char* pattern = "^https?://([^/:]+)";
    
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        strcpy(result, "404 Error: Regex compilation failed");
        return result;
    }
    
    if (regexec(&regex, url, 3, matches, 0) == 0) {
        int start = matches[1].rm_so;
        int end = matches[1].rm_eo;
        char networkLocation[128];
        
        strncpy(networkLocation, url + start, end - start);
        networkLocation[end - start] = '\\0';
        
        if (strcmp(networkLocation, "example.com") == 0) {
            snprintf(result, sizeof(result), "Redirect to: %s", url);
        } else {
            strcpy(result, "404 Error: Domain not allowed");
        }
    } else {
        strcpy(result, "404 Error: Invalid URL");
    }
    
    regfree(&regex);
    return result;
}

int main() {
    // Test cases
    printf("%s\\n", redirect_url("http://example.com/page"));
    printf("%s\\n", redirect_url("https://example.com/admin"));
    printf("%s\\n", redirect_url("http://notexample.com/page"));
    printf("%s\\n", redirect_url("https://evil.com/phishing"));
    printf("%s\\n", redirect_url("http://example.com:8080/secure"));
    
    return 0;
}
