
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** number(char** lines, int size, int* resultSize) {
    *resultSize = size;
    if (size == 0) {
        return NULL;
    }
    
    char** result = (char**)malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        int numDigits = snprintf(NULL, 0, "%d", i + 1);
        int totalLen = numDigits + 2 + strlen(lines[i]) + 1; // number + ": " + line + '\\0'
        result[i] = (char*)malloc(totalLen * sizeof(char));
        sprintf(result[i], "%d: %s", i + 1, lines[i]);
    }
    return result;
}

void freeResult(char** result, int size) {
    for (int i = 0; i < size; i++) {
        free(result[i]);
    }
    free(result);
}

int main() {
    int resultSize;
    
    // Test case 1: Empty list
    char** test1 = NULL;
    char** result1 = number(test1, 0, &resultSize);
    printf("Test 1: []\\n");
    
    // Test case 2: Basic example
    char* test2[] = {"a", "b", "c"};
    char** result2 = number(test2, 3, &resultSize);
    printf("Test 2: [");
    for (int i = 0; i < resultSize; i++) {
        printf("\\"%s\\"", result2[i]);
        if (i < resultSize - 1) printf(", ");
    }
    printf("]\\n");
    freeResult(result2, resultSize);
    
    // Test case 3: Single element
    char* test3[] = {"hello"};
    char** result3 = number(test3, 1, &resultSize);
    printf("Test 3: [");
    for (int i = 0; i < resultSize; i++) {
        printf("\\"%s\\"", result3[i]);
        if (i < resultSize - 1) printf(", ");
    }
    printf("]\\n");
    freeResult(result3, resultSize);
    
    // Test case 4: Multiple words
    char* test4[] = {"hello world", "foo bar", "test"};
    char** result4 = number(test4, 3, &resultSize);
    printf("Test 4: [");
    for (int i = 0; i < resultSize; i++) {
        printf("\\"%s\\"", result4[i]);
        if (i < resultSize - 1) printf(", ");
    }
    printf("]\\n");
    freeResult(result4, resultSize);
    
    // Test case 5: Empty strings
    char* test5[] = {"", "a", ""};
    char** result5 = number(test5, 3, &resultSize);
    printf("Test 5: [");
    for (int i = 0; i < resultSize; i++) {
        printf("\\"%s\\"", result5[i]);
        if (i < resultSize - 1) printf(", ");
    }
    printf("]\\n");
    freeResult(result5, resultSize);
    
    return 0;
}
