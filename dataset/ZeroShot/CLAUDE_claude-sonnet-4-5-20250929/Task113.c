
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* concatenateStrings(char** strings, int count) {
    if (strings == NULL || count <= 0) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    int totalLength = 0;
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            totalLength += strlen(strings[i]);
        }
    }
    
    char* result = (char*)malloc(totalLength + 1);
    if (result == NULL) {
        return NULL;
    }
    
    result[0] = '\\0';
    for (int i = 0; i < count; i++) {
        if (strings[i] != NULL) {
            strcat(result, strings[i]);
        }
    }
    
    return result;
}

int main() {
    // Test case 1: Multiple strings
    char* test1[] = {"Hello", " ", "World", "!"};
    char* result1 = concatenateStrings(test1, 4);
    printf("Test 1: %s\\n", result1);
    free(result1);
    
    // Test case 2: Empty array
    char* result2 = concatenateStrings(NULL, 0);
    printf("Test 2: %s\\n", result2);
    free(result2);
    
    // Test case 3: Single string
    char* test3[] = {"SingleString"};
    char* result3 = concatenateStrings(test3, 1);
    printf("Test 3: %s\\n", result3);
    free(result3);
    
    // Test case 4: Strings with numbers and special characters
    char* test4[] = {"C", "123", "@#$", "Test"};
    char* result4 = concatenateStrings(test4, 4);
    printf("Test 4: %s\\n", result4);
    free(result4);
    
    // Test case 5: Multiple words
    char* test5[] = {"This", " ", "is", " ", "a", " ", "test"};
    char* result5 = concatenateStrings(test5, 7);
    printf("Test 5: %s\\n", result5);
    free(result5);
    
    // Interactive user input
    printf("\\nEnter number of strings to concatenate: ");
    int n;
    scanf("%d", &n);
    getchar(); // consume newline
    
    char** userStrings = (char**)malloc(n * sizeof(char*));
    printf("Enter %d strings:\\n", n);
    for (int i = 0; i < n; i++) {
        userStrings[i] = (char*)malloc(256);
        fgets(userStrings[i], 256, stdin);
        userStrings[i][strcspn(userStrings[i], "\\n")] = '\\0'; // remove newline
    }
    
    char* userResult = concatenateStrings(userStrings, n);
    printf("Result: %s\\n", userResult);
    
    free(userResult);
    for (int i = 0; i < n; i++) {
        free(userStrings[i]);
    }
    free(userStrings);
    
    return 0;
}
