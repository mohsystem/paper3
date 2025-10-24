
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isPangram(const char* sentence) {
    if (sentence == NULL || strlen(sentence) == 0) {
        return false;
    }
    
    bool alphabet[26] = {false};
    
    for (int i = 0; sentence[i] != '\\0'; i++) {
        char c = tolower(sentence[i]);
        if (c >= 'a' && c <= 'z') {
            alphabet[c - 'a'] = true;
        }
    }
    
    for (int i = 0; i < 26; i++) {
        if (!alphabet[i]) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test case 1
    const char* test1 = "The quick brown fox jumps over the lazy dog";
    printf("Test 1: \\"%s\\" -> %s\\n", test1, isPangram(test1) ? "true" : "false");
    
    // Test case 2
    const char* test2 = "Hello World";
    printf("Test 2: \\"%s\\" -> %s\\n", test2, isPangram(test2) ? "true" : "false");
    
    // Test case 3
    const char* test3 = "abcdefghijklmnopqrstuvwxyz";
    printf("Test 3: \\"%s\\" -> %s\\n", test3, isPangram(test3) ? "true" : "false");
    
    // Test case 4
    const char* test4 = "Pack my box with five dozen liquor jugs!";
    printf("Test 4: \\"%s\\" -> %s\\n", test4, isPangram(test4) ? "true" : "false");
    
    // Test case 5
    const char* test5 = "1234567890!@#$%^&*()";
    printf("Test 5: \\"%s\\" -> %s\\n", test5, isPangram(test5) ? "true" : "false");
    
    return 0;
}
