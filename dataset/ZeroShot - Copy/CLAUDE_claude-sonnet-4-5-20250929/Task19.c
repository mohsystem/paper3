
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

void reverseString(char* str, int start, int end) {
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* reverseWords(const char* str) {
    if (str == NULL) {
        return NULL;
    }
    
    int len = strlen(str);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    strcpy(result, str);
    
    int start = 0;
    int i = 0;
    
    while (i <= len) {
        if (result[i] == ' ' || result[i] == '\\0') {
            int wordLen = i - start;
            if (wordLen >= 5) {
                reverseString(result, start, i - 1);
            }
            start = i + 1;
        }
        i++;
    }
    
    return result;
}

int main() {
    // Test cases
    char* result1 = reverseWords("Hey fellow warriors");
    printf("Test 1: %s\\n", result1);
    printf("Expected: Hey wollef sroirraw\\n\\n");
    free(result1);
    
    char* result2 = reverseWords("This is a test");
    printf("Test 2: %s\\n", result2);
    printf("Expected: This is a test\\n\\n");
    free(result2);
    
    char* result3 = reverseWords("This is another test");
    printf("Test 3: %s\\n", result3);
    printf("Expected: This is rehtona test\\n\\n");
    free(result3);
    
    char* result4 = reverseWords("abcde");
    printf("Test 4: %s\\n", result4);
    printf("Expected: edcba\\n\\n");
    free(result4);
    
    char* result5 = reverseWords("a b c d");
    printf("Test 5: %s\\n", result5);
    printf("Expected: a b c d\\n");
    free(result5);
    
    return 0;
}
