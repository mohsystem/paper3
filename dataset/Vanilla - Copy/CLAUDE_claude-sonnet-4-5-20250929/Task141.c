
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* reverseString(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    int len = strlen(input);
    char* reversed = (char*)malloc((len + 1) * sizeof(char));
    
    if (reversed == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < len; i++) {
        reversed[i] = input[len - 1 - i];
    }
    reversed[len] = '\\0';
    
    return reversed;
}

int main() {
    // Test case 1
    const char* test1 = "Hello";
    char* result1 = reverseString(test1);
    printf("Input: %s\\n", test1);
    printf("Output: %s\\n\\n", result1);
    free(result1);
    
    // Test case 2
    const char* test2 = "12345";
    char* result2 = reverseString(test2);
    printf("Input: %s\\n", test2);
    printf("Output: %s\\n\\n", result2);
    free(result2);
    
    // Test case 3
    const char* test3 = "A man a plan a canal Panama";
    char* result3 = reverseString(test3);
    printf("Input: %s\\n", test3);
    printf("Output: %s\\n\\n", result3);
    free(result3);
    
    // Test case 4
    const char* test4 = "";
    char* result4 = reverseString(test4);
    printf("Input: \\"%s\\"\\n", test4);
    printf("Output: \\"%s\\"\\n\\n", result4);
    free(result4);
    
    // Test case 5
    const char* test5 = "Reverse Me!";
    char* result5 = reverseString(test5);
    printf("Input: %s\\n", test5);
    printf("Output: %s\\n", result5);
    free(result5);
    
    return 0;
}
