
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void reverseString(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

char* spinWords(const char* sentence) {
    char* result = (char*)malloc(strlen(sentence) + 1);
    char* copy = (char*)malloc(strlen(sentence) + 1);
    strcpy(copy, sentence);
    
    result[0] = '\\0';
    char* token = strtok(copy, " ");
    int first = 1;
    
    while (token != NULL) {
        if (!first) {
            strcat(result, " ");
        }
        first = 0;
        
        if (strlen(token) >= 5) {
            reverseString(token);
        }
        strcat(result, token);
        
        token = strtok(NULL, " ");
    }
    
    free(copy);
    return result;
}

int main() {
    // Test case 1
    char* result1 = spinWords("Hey fellow warriors");
    printf("%s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = spinWords("This is a test");
    printf("%s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = spinWords("This is another test");
    printf("%s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = spinWords("Welcome");
    printf("%s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = spinWords("Hi");
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
