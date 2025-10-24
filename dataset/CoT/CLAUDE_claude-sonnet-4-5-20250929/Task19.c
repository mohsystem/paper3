
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void reverseString(char* str, int length) {
    int i, j;
    char temp;
    for (i = 0, j = length - 1; i < j; i++, j--) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

char* reverseWords(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    int len = strlen(input);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    strcpy(result, input);
    
    int i = 0;
    while (i < len) {
        int start = i;
        while (i < len && result[i] != ' ') {
            i++;
        }
        
        int wordLen = i - start;
        if (wordLen >= 5) {
            reverseString(result + start, wordLen);
        }
        
        if (i < len) {
            i++;
        }
    }
    
    return result;
}

int main() {
    char* result;
    
    // Test case 1
    result = reverseWords("Hey fellow warriors");
    printf("%s\\n", result);
    free(result);
    
    // Test case 2
    result = reverseWords("This is a test");
    printf("%s\\n", result);
    free(result);
    
    // Test case 3
    result = reverseWords("This is another test");
    printf("%s\\n", result);
    free(result);
    
    // Test case 4
    result = reverseWords("Hello");
    printf("%s\\n", result);
    free(result);
    
    // Test case 5
    result = reverseWords("Hi");
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
