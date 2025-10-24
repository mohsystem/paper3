
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* accum(const char* s) {
    if (s == NULL || strlen(s) == 0) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    size_t len = strlen(s);
    // Calculate maximum possible size
    size_t max_size = 0;
    for (size_t i = 0; i < len; i++) {
        max_size += (i + 1) + 1; // characters + separator
    }
    max_size++; // null terminator
    
    char* result = (char*)malloc(max_size);
    if (result == NULL) {
        return NULL;
    }
    
    size_t pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        
        // Validate input - only letters allowed
        if (!isalpha((unsigned char)c)) {
            continue;
        }
        
        // Add separator if not first element
        if (pos > 0) {
            result[pos++] = '-';
        }
        
        // First character uppercase
        result[pos++] = toupper((unsigned char)c);
        
        // Rest lowercase, repeated i times
        for (size_t j = 0; j < i; j++) {
            result[pos++] = tolower((unsigned char)c);
        }
    }
    
    result[pos] = '\\0';
    return result;
}

int main() {
    char* result;
    
    // Test case 1
    result = accum("abcd");
    printf("Test 1: %s\\n", result);
    free(result);
    
    // Test case 2
    result = accum("RqaEzty");
    printf("Test 2: %s\\n", result);
    free(result);
    
    // Test case 3
    result = accum("cwAt");
    printf("Test 3: %s\\n", result);
    free(result);
    
    // Test case 4
    result = accum("ZpglnRxqenU");
    printf("Test 4: %s\\n", result);
    free(result);
    
    // Test case 5
    result = accum("a");
    printf("Test 5: %s\\n", result);
    free(result);
    
    return 0;
}
