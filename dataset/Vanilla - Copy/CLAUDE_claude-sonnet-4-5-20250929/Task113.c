
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* concatenateStrings(char** strings, int count) {
    int totalLength = 0;
    for (int i = 0; i < count; i++) {
        totalLength += strlen(strings[i]);
    }
    
    char* result = (char*)malloc((totalLength + 1) * sizeof(char));
    result[0] = '\\0';
    
    for (int i = 0; i < count; i++) {
        strcat(result, strings[i]);
    }
    
    return result;
}

int main() {
    // Test case 1
    char* test1[] = {"Hello", " ", "World"};
    char* result1 = concatenateStrings(test1, 3);
    printf("Test 1: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* test2[] = {"Java", "Programming", "Language"};
    char* result2 = concatenateStrings(test2, 3);
    printf("Test 2: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* test3[] = {"One", "Two", "Three", "Four"};
    char* result3 = concatenateStrings(test3, 4);
    printf("Test 3: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* test4[] = {"", "Empty", "", "Strings"};
    char* result4 = concatenateStrings(test4, 4);
    printf("Test 4: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* test5[] = {"Single"};
    char* result5 = concatenateStrings(test5, 1);
    printf("Test 5: %s\\n", result5);
    free(result5);
    
    return 0;
}
