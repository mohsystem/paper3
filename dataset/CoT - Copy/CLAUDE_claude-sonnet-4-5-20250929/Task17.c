
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char* toJadenCase(const char* phrase) {
    if (phrase == NULL || strlen(phrase) == 0) {
        return NULL;
    }
    
    size_t len = strlen(phrase);
    char* result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    int capitalizeNext = 1;
    size_t j = 0;
    
    for (size_t i = 0; i < len; i++) {
        char c = phrase[i];
        if (isspace((unsigned char)c)) {
            result[j++] = c;
            capitalizeNext = 1;
        } else if (capitalizeNext) {
            result[j++] = toupper((unsigned char)c);
            capitalizeNext = 0;
        } else {
            result[j++] = c;
        }
    }
    
    result[j] = '\\0';
    return result;
}

int main() {
    char* result1 = toJadenCase("How can mirrors be real if our eyes aren't real");
    printf("Test 1: %s\\n", result1 ? result1 : "NULL");
    free(result1);
    
    char* result2 = toJadenCase("this is a test");
    printf("Test 2: %s\\n", result2 ? result2 : "NULL");
    free(result2);
    
    char* result3 = toJadenCase("i love programming");
    printf("Test 3: %s\\n", result3 ? result3 : "NULL");
    free(result3);
    
    char* result4 = toJadenCase(NULL);
    printf("Test 4: %s\\n", result4 ? result4 : "NULL");
    
    char* result5 = toJadenCase("");
    printf("Test 5: %s\\n", result5 ? result5 : "NULL");
    
    return 0;
}
