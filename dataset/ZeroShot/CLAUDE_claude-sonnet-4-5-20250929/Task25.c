
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** number(char** lines, int size, int* result_size) {
    if (lines == NULL || size < 0) {
        *result_size = 0;
        return NULL;
    }
    
    char** result = (char**)malloc(size * sizeof(char*));
    if (result == NULL) {
        *result_size = 0;
        return NULL;
    }
    
    *result_size = size;
    
    for (int i = 0; i < size; i++) {
        int num_digits = snprintf(NULL, 0, "%d", i + 1);
        int total_length = num_digits + 2 + strlen(lines[i]) + 1;
        
        result[i] = (char*)malloc(total_length * sizeof(char));
        if (result[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *result_size = 0;
            return NULL;
        }
        
        snprintf(result[i], total_length, "%d: %s", i + 1, lines[i]);
    }
    
    return result;
}

void free_result(char** result, int size) {
    if (result != NULL) {
        for (int i = 0; i < size; i++) {
            free(result[i]);
        }
        free(result);
    }
}

int main() {
    int result_size;
    
    // Test case 1: Empty list
    printf("Test 1: ");
    char** test1 = number(NULL, 0, &result_size);
    printf("Empty list\\n");
    free_result(test1, result_size);
    
    // Test case 2: Simple list
    printf("Test 2: ");
    char* lines2[] = {"a", "b", "c"};
    char** test2 = number(lines2, 3, &result_size);
    for (int i = 0; i < result_size; i++) {
        printf("%s ", test2[i]);
    }
    printf("\\n");
    free_result(test2, result_size);
    
    // Test case 3: Single element
    printf("Test 3: ");
    char* lines3[] = {"Hello World"};
    char** test3 = number(lines3, 1, &result_size);
    for (int i = 0; i < result_size; i++) {
        printf("%s ", test3[i]);
    }
    printf("\\n");
    free_result(test3, result_size);
    
    // Test case 4: Multiple lines
    printf("Test 4: ");
    char* lines4[] = {"line one", "line two", "line three", "line four"};
    char** test4 = number(lines4, 4, &result_size);
    for (int i = 0; i < result_size; i++) {
        printf("%s ", test4[i]);
    }
    printf("\\n");
    free_result(test4, result_size);
    
    // Test case 5: Lines with special characters
    printf("Test 5: ");
    char* lines5[] = {"Hello!", "How are you?", "I'm fine, thanks."};
    char** test5 = number(lines5, 3, &result_size);
    for (int i = 0; i < result_size; i++) {
        printf("%s ", test5[i]);
    }
    printf("\\n");
    free_result(test5, result_size);
    
    return 0;
}
