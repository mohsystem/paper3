#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Takes an array of strings and returns a new array with each line prepended 
 * by the correct number. The numbering starts at 1. The format is n: string.
 * NOTE: The caller is responsible for freeing the returned array and each string within it.
 * @param lines An array of C-style strings.
 * @param count The number of strings in the array.
 * @return A new dynamically allocated array of strings with line numbers, or NULL on failure or if count is 0.
 */
char** number(const char** lines, size_t count) {
    if (count == 0) {
        return NULL;
    }

    char** result = (char**)malloc(count * sizeof(char*));
    if (result == NULL) {
        perror("Failed to allocate memory for result array");
        return NULL;
    }

    for (size_t i = 0; i < count; ++i) {
        int len = snprintf(NULL, 0, "%zu: %s", i + 1, lines[i]);
        if (len < 0) {
            fprintf(stderr, "Encoding error\n");
            for (size_t j = 0; j < i; ++j) free(result[j]);
            free(result);
            return NULL;
        }

        result[i] = (char*)malloc(len + 1);
        if (result[i] == NULL) {
            perror("Failed to allocate memory for a line");
            for (size_t j = 0; j < i; ++j) free(result[j]);
            free(result);
            return NULL;
        }
        
        snprintf(result[i], len + 1, "%zu: %s", i + 1, lines[i]);
    }

    return result;
}

void free_lines(char** lines, size_t count) {
    if (lines == NULL) {
        return;
    }
    for (size_t i = 0; i < count; ++i) {
        free(lines[i]);
    }
    free(lines);
}

void run_test_case(const char* test_name, const char** input, size_t count) {
    printf("%s\n", test_name);
    printf("Input: [");
    for(size_t i = 0; i < count; i++) {
        printf("\"%s\"", input[i]);
        if (i < count - 1) printf(", ");
    }
    printf("]\n");

    char** output = number(input, count);
    
    printf("Output: [");
    if (output != NULL) {
        for(size_t i = 0; i < count; i++) {
            printf("\"%s\"", output[i]);
            if (i < count - 1) printf(", ");
        }
    }
    printf("]\n\n");
    
    free_lines(output, count);
}

int main() {
    // Test Case 1: Basic case
    const char* input1[] = {"a", "b", "c"};
    run_test_case("Test Case 1", input1, sizeof(input1)/sizeof(input1[0]));

    // Test Case 2: Empty list
    const char* input2[] = {};
    run_test_case("Test Case 2", input2, 0);

    // Test Case 3: List with empty strings
    const char* input3[] = {"", "x", ""};
    run_test_case("Test Case 3", input3, sizeof(input3)/sizeof(input3[0]));

    // Test Case 4: List with multi-word strings
    const char* input4[] = {"hello world", "goodbye world"};
    run_test_case("Test Case 4", input4, sizeof(input4)/sizeof(input4[0]));
    
    // Test Case 5: Single element list
    const char* input5[] = {"Just one line."};
    run_test_case("Test Case 5", input5, sizeof(input5)/sizeof(input5[0]));

    return 0;
}