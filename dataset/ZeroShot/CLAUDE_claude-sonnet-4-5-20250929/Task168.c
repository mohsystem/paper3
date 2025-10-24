
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* tweakLetters(const char* str, int* tweaks, int length) {
    if (str == NULL || tweaks == NULL || strlen(str) != length) {
        char* result = (char*)malloc((strlen(str) + 1) * sizeof(char));
        strcpy(result, str);
        return result;
    }
    
    char* result = (char*)malloc((length + 1) * sizeof(char));
    
    for (int i = 0; i < length; i++) {
        char currentChar = str[i];
        int tweak = tweaks[i];
        
        // Apply the tweak to the character
        char newChar = currentChar + tweak;
        
        // Handle wrapping for lowercase letters
        if (currentChar >= 'a' && currentChar <= 'z') {
            if (newChar > 'z') {
                newChar = 'a' + (newChar - 'z' - 1);
            } else if (newChar < 'a') {
                newChar = 'z' - ('a' - newChar - 1);
            }
        }
        
        result[i] = newChar;
    }
    
    result[length] = '\\0';
    return result;
}

int main() {
    // Test case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    char* result1 = tweakLetters("apple", tweaks1, 5);
    printf("%s\\n", result1);
    free(result1);
    
    // Test case 2
    int tweaks2[] = {0, 0, 0, -1};
    char* result2 = tweakLetters("many", tweaks2, 4);
    printf("%s\\n", result2);
    free(result2);
    
    // Test case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    char* result3 = tweakLetters("rhino", tweaks3, 5);
    printf("%s\\n", result3);
    free(result3);
    
    // Test case 4
    int tweaks4[] = {1, 0, 0, 0, -1};
    char* result4 = tweakLetters("zebra", tweaks4, 5);
    printf("%s\\n", result4);
    free(result4);
    
    // Test case 5
    int tweaks5[] = {0, 0, 0, 0, 0};
    char* result5 = tweakLetters("hello", tweaks5, 5);
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
