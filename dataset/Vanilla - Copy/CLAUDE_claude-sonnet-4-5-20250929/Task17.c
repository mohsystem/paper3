
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* toJadenCase(const char* phrase) {
    if (phrase == NULL || strlen(phrase) == 0) {
        return NULL;
    }
    
    int len = strlen(phrase);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    
    if (result == NULL) {
        return NULL;
    }
    
    int capitalizeNext = 1;
    
    for (int i = 0; i < len; i++) {
        char c = phrase[i];
        
        if (isspace(c)) {
            result[i] = c;
            capitalizeNext = 1;
        } else {
            if (capitalizeNext && isalpha(c)) {
                result[i] = toupper(c);
                capitalizeNext = 0;
            } else {
                result[i] = c;
            }
        }
    }
    
    result[len] = '\\0';
    return result;
}

int main() {
    // Test cases
    char* result1 = toJadenCase("How can mirrors be real if our eyes aren't real");
    if (result1) {
        printf("%s\\n", result1);
        free(result1);
    }
    
    char* result2 = toJadenCase("the quick brown fox jumps over the lazy dog");
    if (result2) {
        printf("%s\\n", result2);
        free(result2);
    }
    
    char* result3 = toJadenCase("i love programming");
    if (result3) {
        printf("%s\\n", result3);
        free(result3);
    }
    
    char* result4 = toJadenCase("");
    if (result4) {
        printf("%s\\n", result4);
        free(result4);
    } else {
        printf("NULL\\n");
    }
    
    char* result5 = toJadenCase(NULL);
    if (result5) {
        printf("%s\\n", result5);
        free(result5);
    } else {
        printf("NULL\\n");
    }
    
    return 0;
}
