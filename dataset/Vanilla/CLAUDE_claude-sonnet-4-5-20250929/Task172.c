
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LEN 2001
#define MAX_SUBSTRINGS 4000000

typedef struct {
    char str[MAX_LEN];
} Substring;

bool areEqual(const char* text, int start1, int start2, int len) {
    for (int i = 0; i < len; i++) {
        if (text[start1 + i] != text[start2 + i]) {
            return false;
        }
    }
    return true;
}

bool containsSubstring(Substring* set, int size, const char* text, int start, int len) {
    for (int i = 0; i < size; i++) {
        if (strlen(set[i].str) == len) {
            bool match = true;
            for (int j = 0; j < len; j++) {
                if (set[i].str[j] != text[start + j]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
    }
    return false;
}

int distinctEchoSubstrings(const char* text) {
    static Substring distinctSubstrings[MAX_SUBSTRINGS];
    int count = 0;
    int n = strlen(text);
    
    // Try all possible substring lengths (must be even)
    for (int len = 2; len <= n; len += 2) {
        int halfLen = len / 2;
        
        // Try all possible starting positions
        for (int i = 0; i <= n - len; i++) {
            if (areEqual(text, i, i + halfLen, halfLen)) {
                if (!containsSubstring(distinctSubstrings, count, text, i, len)) {
                    for (int j = 0; j < len; j++) {
                        distinctSubstrings[count].str[j] = text[i + j];
                    }
                    distinctSubstrings[count].str[len] = '\\0';
                    count++;
                }
            }
        }
    }
    
    return count;
}

int main() {
    // Test case 1
    const char* test1 = "abcabcabc";
    printf("Input: \\"%s\\"\\n", test1);
    printf("Output: %d\\n\\n", distinctEchoSubstrings(test1));
    
    // Test case 2
    const char* test2 = "leetcodeleetcode";
    printf("Input: \\"%s\\"\\n", test2);
    printf("Output: %d\\n\\n", distinctEchoSubstrings(test2));
    
    // Test case 3
    const char* test3 = "aaaa";
    printf("Input: \\"%s\\"\\n", test3);
    printf("Output: %d\\n\\n", distinctEchoSubstrings(test3));
    
    // Test case 4
    const char* test4 = "abcdef";
    printf("Input: \\"%s\\"\\n", test4);
    printf("Output: %d\\n\\n", distinctEchoSubstrings(test4));
    
    // Test case 5
    const char* test5 = "aabbccdd";
    printf("Input: \\"%s\\"\\n", test5);
    printf("Output: %d\\n", distinctEchoSubstrings(test5));
    
    return 0;
}
