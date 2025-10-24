
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isPangram(const char* text) {
    if (text == NULL || strlen(text) == 0) {
        return false;
    }
    
    bool alphabet[26] = {false};
    int count = 0;
    
    for (size_t i = 0; i < strlen(text); i++) {
        char c = tolower((unsigned char)text[i]);
        if (c >= 'a' && c <= 'z') {
            int index = c - 'a';
            if (!alphabet[index]) {
                alphabet[index] = true;
                count++;
            }
        }
    }
    
    return count == 26;
}

int main() {
    // Test case 1: Classic pangram
    printf("Test 1: %s\\n", isPangram("The quick brown fox jumps over the lazy dog") ? "true" : "false");
    
    // Test case 2: Pangram with numbers and punctuation
    printf("Test 2: %s\\n", isPangram("Pack my box with five dozen liquor jugs!123") ? "true" : "false");
    
    // Test case 3: Not a pangram
    printf("Test 3: %s\\n", isPangram("Hello World") ? "true" : "false");
    
    // Test case 4: Empty string
    printf("Test 4: %s\\n", isPangram("") ? "true" : "false");
    
    // Test case 5: All letters with special characters
    printf("Test 5: %s\\n", isPangram("abcdefghijklmnopqrstuvwxyz!!!") ? "true" : "false");
    
    return 0;
}
