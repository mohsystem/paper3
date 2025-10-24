
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

char* longest(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    bool seen[26];
    memset(seen, false, sizeof(seen));
    
    for (size_t i = 0; s1[i] != '\\0'; i++) {
        char c = s1[i];
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }
    
    for (size_t i = 0; s2[i] != '\\0'; i++) {
        char c = s2[i];
        if (c >= 'a' && c <= 'z') {
            seen[c - 'a'] = true;
        }
    }
    
    char* result = (char*)malloc(27);
    int pos = 0;
    
    for (int i = 0; i < 26; i++) {
        if (seen[i]) {
            result[pos++] = 'a' + i;
        }
    }
    result[pos] = '\\0';
    
    return result;
}

int main() {
    // Test case 1
    const char* a1 = "xyaabbbccccdefww";
    const char* b1 = "xxxxyyyyabklmopq";
    char* res1 = longest(a1, b1);
    printf("Test 1: %s\\n", res1);
    free(res1);
    
    // Test case 2
    const char* a2 = "abcdefghijklmnopqrstuvwxyz";
    char* res2 = longest(a2, a2);
    printf("Test 2: %s\\n", res2);
    free(res2);
    
    // Test case 3
    const char* a3 = "abc";
    const char* b3 = "xyz";
    char* res3 = longest(a3, b3);
    printf("Test 3: %s\\n", res3);
    free(res3);
    
    // Test case 4
    const char* a4 = "zzz";
    const char* b4 = "aaa";
    char* res4 = longest(a4, b4);
    printf("Test 4: %s\\n", res4);
    free(res4);
    
    // Test case 5
    const char* a5 = "";
    const char* b5 = "abc";
    char* res5 = longest(a5, b5);
    printf("Test 5: %s\\n", res5);
    free(res5);
    
    return 0;
}
