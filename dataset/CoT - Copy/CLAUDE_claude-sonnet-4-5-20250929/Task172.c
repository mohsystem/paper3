
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_SUBSTRINGS 4000

typedef struct {
    char** strings;
    int size;
    int capacity;
} StringSet;

StringSet* createSet() {
    StringSet* set = (StringSet*)malloc(sizeof(StringSet));
    set->capacity = MAX_SUBSTRINGS;
    set->size = 0;
    set->strings = (char**)malloc(set->capacity * sizeof(char*));
    return set;
}

int contains(StringSet* set, const char* str) {
    for (int i = 0; i < set->size; i++) {
        if (strcmp(set->strings[i], str) == 0) {
            return 1;
        }
    }
    return 0;
}

void addToSet(StringSet* set, const char* str) {
    if (!contains(set, str)) {
        set->strings[set->size] = (char*)malloc((strlen(str) + 1) * sizeof(char));
        strcpy(set->strings[set->size], str);
        set->size++;
    }
}

void freeSet(StringSet* set) {
    for (int i = 0; i < set->size; i++) {
        free(set->strings[i]);
    }
    free(set->strings);
    free(set);
}

int distinctEchoSubstrings(const char* text) {
    // Input validation
    if (text == NULL || strlen(text) == 0 || strlen(text) > 2000) {
        return 0;
    }
    
    // Validate that text contains only lowercase English letters
    for (int i = 0; text[i] != '\\0'; i++) {
        if (text[i] < 'a' || text[i] > 'z') {
            return 0;
        }
    }
    
    StringSet* distinctSubstrings = createSet();
    int n = strlen(text);
    
    // Iterate through all possible substring lengths (must be even)
    for (int len = 2; len <= n; len += 2) {
        int halfLen = len / 2;
        
        // Iterate through all possible starting positions
        for (int i = 0; i <= n - len; i++) {
            // Extract and compare the two halves
            int match = 1;
            for (int j = 0; j < halfLen; j++) {
                if (text[i + j] != text[i + halfLen + j]) {
                    match = 0;
                    break;
                }
            }
            
            if (match) {
                char* substring = (char*)malloc((len + 1) * sizeof(char));
                strncpy(substring, text + i, len);
                substring[len] = '\\0';
                addToSet(distinctSubstrings, substring);
                free(substring);
            }
        }
    }
    
    int result = distinctSubstrings->size;
    freeSet(distinctSubstrings);
    return result;
}

int main() {
    // Test case 1
    printf("Test 1: %d\\n", distinctEchoSubstrings("abcabcabc")); // Expected: 3
    
    // Test case 2
    printf("Test 2: %d\\n", distinctEchoSubstrings("leetcodeleetcode")); // Expected: 2
    
    // Test case 3
    printf("Test 3: %d\\n", distinctEchoSubstrings("aa")); // Expected: 1
    
    // Test case 4
    printf("Test 4: %d\\n", distinctEchoSubstrings("a")); // Expected: 0
    
    // Test case 5
    printf("Test 5: %d\\n", distinctEchoSubstrings("aaaa")); // Expected: 2
    
    return 0;
}
