
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

bool isPangram(const char* sentence) {
    if (sentence == NULL || strlen(sentence) == 0) {
        return false;
    }
    
    bool letters[26] = {false};
    int count = 0;
    
    for (int i = 0; sentence[i] != '\\0'; i++) {
        if (isalpha(sentence[i])) {
            char lower = tolower(sentence[i]);
            int index = lower - 'a';
            
            if (!letters[index]) {
                letters[index] = true;
                count++;
            }
        }
    }
    
    return count == 26;
}

int main() {
    // Test cases
    printf("Test 1: %s\\n", isPangram("The quick brown fox jumps over the lazy dog") ? "true" : "false");
    printf("Test 2: %s\\n", isPangram("Hello World") ? "true" : "false");
    printf("Test 3: %s\\n", isPangram("abcdefghijklmnopqrstuvwxyz") ? "true" : "false");
    printf("Test 4: %s\\n", isPangram("Pack my box with five dozen liquor jugs") ? "true" : "false");
    printf("Test 5: %s\\n", isPangram("This is not a pangram sentence") ? "true" : "false");
    
    return 0;
}
