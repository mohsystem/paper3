
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* longest(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    int charCount[26] = {0};
    
    for (int i = 0; s1[i] != '\\0'; i++) {
        if (s1[i] >= 'a' && s1[i] <= 'z') {
            charCount[s1[i] - 'a'] = 1;
        }
    }
    
    for (int i = 0; s2[i] != '\\0'; i++) {
        if (s2[i] >= 'a' && s2[i] <= 'z') {
            charCount[s2[i] - 'a'] = 1;
        }
    }
    
    char* result = (char*)malloc(27 * sizeof(char));
    int index = 0;
    
    for (int i = 0; i < 26; i++) {
        if (charCount[i] == 1) {
            result[index++] = 'a' + i;
        }
    }
    result[index] = '\\0';
    
    return result;
}

int main() {
    // Test case 1
    const char* a1 = "xyaabbbccccdefww";
    const char* b1 = "xxxxyyyyabklmopq";
    char* result1 = longest(a1, b1);
    printf("Test 1: %s\\n", result1);
    free(result1);
    
    // Test case 2
    const char* a2 = "abcdefghijklmnopqrstuvwxyz";
    const char* b2 = "abcdefghijklmnopqrstuvwxyz";
    char* result2 = longest(a2, b2);
    printf("Test 2: %s\\n", result2);
    free(result2);
    
    // Test case 3
    const char* a3 = "aretheyhere";
    const char* b3 = "yestheyarehere";
    char* result3 = longest(a3, b3);
    printf("Test 3: %s\\n", result3);
    free(result3);
    
    // Test case 4
    const char* a4 = "loopingisfunbutdangerous";
    const char* b4 = "lessdangerousthancoding";
    char* result4 = longest(a4, b4);
    printf("Test 4: %s\\n", result4);
    free(result4);
    
    // Test case 5
    const char* a5 = "";
    const char* b5 = "abc";
    char* result5 = longest(a5, b5);
    printf("Test 5: %s\\n", result5);
    free(result5);
    
    return 0;
}
