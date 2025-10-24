
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* tweakLetters(const char* str, const int* tweaks, int tweaksSize) {
    // Input validation
    if (str == NULL || tweaks == NULL) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    size_t strLen = strlen(str);
    if (strLen == 0) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\\0';
        }
        return empty;
    }
    
    // Ensure arrays have matching lengths
    size_t length = strLen < (size_t)tweaksSize ? strLen : (size_t)tweaksSize;
    
    // Allocate result string
    char* result = (char*)malloc(length + 1);
    if (result == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < length; i++) {
        char c = str[i];
        int tweak = tweaks[i];
        
        // Validate tweak value is within safe range
        if (tweak < -25 || tweak > 25) {
            result[i] = c;
            continue;
        }
        
        // Only process lowercase letters
        if (c >= 'a' && c <= 'z') {
            // Calculate new character with wrapping
            int offset = c - 'a';
            int newOffset = (offset + tweak) % 26;
            // Handle negative modulo
            if (newOffset < 0) {
                newOffset += 26;
            }
            result[i] = (char)('a' + newOffset);
        } else {
            // Non-lowercase letters remain unchanged
            result[i] = c;
        }
    }
    
    result[length] = '\\0';
    return result;
}

int main() {
    char* result;
    
    // Test case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    result = tweakLetters("apple", tweaks1, 5);
    printf("%s\\n", result);
    free(result);
    
    // Test case 2
    int tweaks2[] = {0, 0, 0, -1};
    result = tweakLetters("many", tweaks2, 4);
    printf("%s\\n", result);
    free(result);
    
    // Test case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    result = tweakLetters("rhino", tweaks3, 5);
    printf("%s\\n", result);
    free(result);
    
    // Test case 4
    int tweaks4[] = {1, -1, 0, 1, -1};
    result = tweakLetters("zebra", tweaks4, 5);
    printf("%s\\n", result);
    free(result);
    
    // Test case 5
    int tweaks5[] = {-1, 1, -1, 1};
    result = tweakLetters("code", tweaks5, 4);
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
