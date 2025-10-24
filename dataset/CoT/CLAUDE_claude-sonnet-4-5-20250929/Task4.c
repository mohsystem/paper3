
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isIsogram(const char* str) {
    if (str == NULL) {
        return true;
    }
    
    int seen[26] = {0};
    size_t len = strlen(str);
    
    for (size_t i = 0; i < len; i++) {
        char c = tolower((unsigned char)str[i]);
        if (isalpha((unsigned char)c)) {
            int index = c - 'a';
            if (seen[index]) {
                return false;
            }
            seen[index] = 1;
        }
    }
    
    return true;
}

int main() {
    // Test case 1
    printf("Test 1: 'Dermatoglyphics' -> %s\\n", isIsogram("Dermatoglyphics") ? "true" : "false");
    
    // Test case 2
    printf("Test 2: 'aba' -> %s\\n", isIsogram("aba") ? "true" : "false");
    
    // Test case 3
    printf("Test 3: 'moOse' -> %s\\n", isIsogram("moOse") ? "true" : "false");
    
    // Test case 4
    printf("Test 4: '' -> %s\\n", isIsogram("") ? "true" : "false");
    
    // Test case 5
    printf("Test 5: 'isogram' -> %s\\n", isIsogram("isogram") ? "true" : "false");
    
    return 0;
}
