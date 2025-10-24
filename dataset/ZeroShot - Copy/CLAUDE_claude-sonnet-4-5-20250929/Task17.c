
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
        } else if (capitalizeNext) {
            result[i] = toupper(c);
            capitalizeNext = 0;
        } else {
            result[i] = tolower(c);
        }
    }
    
    result[len] = '\\0';
    return result;
}

int main() {
    // Test cases
    char* test1 = toJadenCase("How can mirrors be real if our eyes aren't real");
    printf("Test 1: %s\\n", test1);
    free(test1);
    
    char* test2 = toJadenCase("the quick brown fox jumps over the lazy dog");
    printf("Test 2: %s\\n", test2);
    free(test2);
    
    char* test3 = toJadenCase("I'm a single word");
    printf("Test 3: %s\\n", test3);
    free(test3);
    
    char* test4 = toJadenCase(NULL);
    printf("Test 4: %s\\n", test4 == NULL ? "NULL" : test4);
    
    char* test5 = toJadenCase("");
    printf("Test 5: %s\\n", test5 == NULL ? "NULL" : test5);
    
    return 0;
}
