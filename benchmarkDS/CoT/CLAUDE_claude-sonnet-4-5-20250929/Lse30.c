
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

#define ROOT_REDIRECT "/"
#define MAX_URL_LENGTH 2048

int urlDecode(const char* src, char* dest, size_t destSize) {
    size_t i = 0, j = 0;
    while (src[i] && j < destSize - 1) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            int value;
            if (sscanf(src + i + 1, "%2x", &value) == 1) {
                dest[j++] = (char)value;
                i += 3;
            } else {
                dest[j++] = src[i++];
            }
        } else if (src[i] == '+') {
            dest[j++] = ' ';
            i++;
        } else {
            dest[j++] = src[i++];
        }
    }
    dest[j] = '\\0';
    return 0;
}

const char* getRedirectUrl(const char* target, char* result, size_t resultSize) {
    if (target == NULL || strlen(target) == 0) {
        strncpy(result, ROOT_REDIRECT, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    char decodedTarget[MAX_URL_LENGTH];
    urlDecode(target, decodedTarget, sizeof(decodedTarget));
    
    // Security checks
    if (strstr(decodedTarget, "http://") == decodedTarget || 
        strstr(decodedTarget, "https://") == decodedTarget || 
        strstr(decodedTarget, "//") == decodedTarget || 
        strchr(decodedTarget, '@') != NULL) {
        strncpy(result, ROOT_REDIRECT, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    // Check regex pattern
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9-]+\\\\.", REG_EXTENDED);
    if (reti) {
        strncpy(result, ROOT_REDIRECT, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    reti = regexec(&regex, decodedTarget, 0, NULL, 0);
    regfree(&regex);
    
    if (reti == 0) {
        // Check for path traversal
        if (strstr(decodedTarget, "..") != NULL || strstr(decodedTarget, "./") != NULL) {
            strncpy(result, ROOT_REDIRECT, resultSize - 1);
            result[resultSize - 1] = '\\0';
            return result;
        }
        strncpy(result, decodedTarget, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return result;
    }
    
    strncpy(result, ROOT_REDIRECT, resultSize - 1);
    result[resultSize - 1] = '\\0';
    return result;
}

int main() {
    char result[MAX_URL_LENGTH];
    
    // Test cases
    printf("Test 1: %s\\n", getRedirectUrl("page1.html", result, sizeof(result)));
    printf("Test 2: %s\\n", getRedirectUrl("my-page.html", result, sizeof(result)));
    printf("Test 3: %s\\n", getRedirectUrl("http://evil.com", result, sizeof(result)));
    printf("Test 4: %s\\n", getRedirectUrl("../etc/passwd", result, sizeof(result)));
    printf("Test 5: %s\\n", getRedirectUrl("invalid", result, sizeof(result)));
    
    return 0;
}
