
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

char** number(char** lines, int count, int* result_count) {
    if (lines == NULL || count < 0) {
        *result_count = 0;
        return NULL;
    }
    
    char** result = (char**)malloc(count * sizeof(char*));
    if (result == NULL) {
        *result_count = 0;
        return NULL;
    }
    
    for (int i = 0; i < count; i++) {
        int line_length = (lines[i] != NULL) ? strlen(lines[i]) : 0;
        int num_length = snprintf(NULL, 0, "%d", i + 1);
        int total_length = num_length + 2 + line_length + 1;
        
        result[i] = (char*)malloc(total_length * sizeof(char));
        if (result[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_count = 0;
            return NULL;
        }
        
        if (lines[i] != NULL) {
            snprintf(result[i], total_length, "%d: %s", i + 1, lines[i]);
        } else {
            snprintf(result[i], total_length, "%d: ", i + 1);
        }
    }
    
    *result_count = count;
    return result;
}

void free_result(char** result, int count) {
    if (result != NULL) {
        for (int i = 0; i < count; i++) {
            free(result[i]);
        }
        free(result);
    }
}

int main() {
    int result_count;
    
    // Test case 1: Empty list
    printf("Test 1: ");
    char** result1 = number(NULL, 0, &result_count);
    printf("Empty\\n");
    
    // Test case 2: Basic example
    printf("Test 2: ");
    char* test2[] = {"a", "b", "c"};
    char** result2 = number(test2, 3, &result_count);
    for (int i = 0; i < result_count; i++) {
        printf("%s ", result2[i]);
    }
    printf("\\n");
    free_result(result2, result_count);
    
    // Test case 3: Single element
    printf("Test 3: ");
    char* test3[] = {"hello"};
    char** result3 = number(test3, 1, &result_count);
    for (int i = 0; i < result_count; i++) {
        printf("%s ", result3[i]);
    }
    printf("\\n");
    free_result(result3, result_count);
    
    // Test case 4: Strings with special characters
    printf("Test 4: ");
    char* test4[] = {"Hello, World!", "Line with spaces", "Special @#$%"};
    char** result4 = number(test4, 3, &result_count);
    for (int i = 0; i < result_count; i++) {
        printf("%s ", result4[i]);
    }
    printf("\\n");
    free_result(result4, result_count);
    
    // Test case 5: Empty strings
    printf("Test 5: ");
    char* test5[] = {"", "non-empty", ""};
    char** result5 = number(test5, 3, &result_count);
    for (int i = 0; i < result_count; i++) {
        printf("%s ", result5[i]);
    }
    printf("\\n");
    free_result(result5, result_count);
    
    return 0;
}
