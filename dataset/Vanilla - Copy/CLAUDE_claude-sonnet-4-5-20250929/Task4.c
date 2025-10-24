
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isIsogram(const char* str) {
    if (str == NULL || str[0] == '\\0') {
        return true;
    }
    
    int charCount[26] = {0};
    
    for (int i = 0; str[i] != '\\0'; i++) {
        char c = tolower(str[i]);
        int index = c - 'a';
        
        if (charCount[index] > 0) {
            return false;
        }
        charCount[index]++;
    }
    
    return true;
}

int main() {
    // Test cases
    printf("Test 1: 'Dermatoglyphics' -> %s\\n", isIsogram("Dermatoglyphics") ? "true" : "false");
    printf("Test 2: 'aba' -> %s\\n", isIsogram("aba") ? "true" : "false");
    printf("Test 3: 'moOse' -> %s\\n", isIsogram("moOse") ? "true" : "false");
    printf("Test 4: '' -> %s\\n", isIsogram("") ? "true" : "false");
    printf("Test 5: 'isogram' -> %s\\n", isIsogram("isogram") ? "true" : "false");
    
    return 0;
}
