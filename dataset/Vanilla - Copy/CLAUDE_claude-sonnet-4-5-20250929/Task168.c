
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* tweakLetters(const char* str, int* tweaks, int length) {
    char* result = (char*)malloc((length + 1) * sizeof(char));
    for (int i = 0; i < length; i++) {
        result[i] = str[i] + tweaks[i];
    }
    result[length] = '\\0';
    return result;
}

int main() {
    char* result;
    
    // Test case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    result = tweakLetters("apple", tweaks1, 5);
    printf("%s\\n", result);  // "aqold"
    free(result);
    
    // Test case 2
    int tweaks2[] = {0, 0, 0, -1};
    result = tweakLetters("many", tweaks2, 4);
    printf("%s\\n", result);  // "manx"
    free(result);
    
    // Test case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    result = tweakLetters("rhino", tweaks3, 5);
    printf("%s\\n", result);  // "sijop"
    free(result);
    
    // Test case 4
    int tweaks4[] = {0, 0, 0, 0, 0};
    result = tweakLetters("hello", tweaks4, 5);
    printf("%s\\n", result);  // "hello"
    free(result);
    
    // Test case 5
    int tweaks5[] = {-1, 1, -1, 1};
    result = tweakLetters("code", tweaks5, 4);
    printf("%s\\n", result);  // "bndf"
    free(result);
    
    return 0;
}
