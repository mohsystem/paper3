
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Process string by trimming trailing whitespace and newlines */
/* Returns new allocated string that must be freed by caller */
char* process_string(const char* input) {
    /* Validate input is not NULL */
    if (input == NULL) {
        return NULL;
    }
    
    /* Validate length to prevent excessive memory usage */
    const size_t MAX_LENGTH = 1048576; /* 1MB limit */
    size_t input_len = strlen(input);
    if (input_len > MAX_LENGTH) {
        fprintf(stderr, "Error: Input exceeds maximum allowed length\\n");
        return NULL;
    }
    
    /* Allocate buffer with bounds check for integer overflow */
    if (input_len >= SIZE_MAX - 1) {
        fprintf(stderr, "Error: Input length too large\\n");
        return NULL;
    }
    
    char* result = (char*)malloc(input_len + 1);
    if (result == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    /* Copy input to result with bounds checking */
    strncpy(result, input, input_len);
    result[input_len] = '\\0'; /* Ensure null termination */
    
    size_t length = input_len;
    
    /* First loop: trim trailing whitespace */
    /* Iterate from end to find last non-whitespace character */
    while (length > 0 && isspace((unsigned char)result[length - 1])) {
        length--;
    }
    
    /* Second loop: trim trailing newlines (redundant but follows specification) */
    while (length > 0 && (result[length - 1] == '\\n' || result[length - 1] == '\\r')) {
        length--;
    }
    
    /* Null terminate the string at new length */
    result[length] = '\\0';
    
    return result;
}

int main(int argc, char* argv[]) {
    /* Check that exactly one argument is provided */
    if (argc != 2) {
        fprintf(stderr, "Error: Exactly one argument required\\n");
        return 1;
    }
    
    /* Check the argument is not NULL (defensive check) */
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: Argument must be a valid string\\n");
        return 1;
    }
    
    /* Process the string */
    char* result = process_string(argv[1]);
    if (result == NULL) {
        fprintf(stderr, "Error: Failed to process string\\n");
        return 1;
    }
    
    printf("Result: '%s'\\n", result);
    free(result);
    result = NULL;
    
    /* Test cases */
    printf("\\n=== Test Cases ===\\n");
    
    /* Test 1: String with trailing spaces */
    const char* test1 = "hello world   ";
    printf("Test 1 Input: '%s'\\n", test1);
    char* out1 = process_string(test1);
    if (out1) {
        printf("Test 1 Output: '%s'\\n", out1);
        free(out1);
        out1 = NULL;
    }
    
    /* Test 2: String with trailing newlines */
    const char* test2 = "hello world\\n\\n";
    printf("Test 2 Input: 'hello world\\\\n\\\\n'\\n");
    char* out2 = process_string(test2);
    if (out2) {
        printf("Test 2 Output: '%s'\\n", out2);
        free(out2);
        out2 = NULL;
    }
    
    /* Test 3: String with mixed trailing whitespace */
    const char* test3 = "hello world \\n \\t ";
    printf("Test 3 Input: 'hello world \\\\n \\\\t '\\n");
    char* out3 = process_string(test3);
    if (out3) {
        printf("Test 3 Output: '%s'\\n", out3);
        free(out3);
        out3 = NULL;
    }
    
    /* Test 4: String with no trailing whitespace */
    const char* test4 = "hello world";
    printf("Test 4 Input: '%s'\\n", test4);
    char* out4 = process_string(test4);
    if (out4) {
        printf("Test 4 Output: '%s'\\n", out4);
        free(out4);
        out4 = NULL;
    }
    
    /* Test 5: Empty string */
    const char* test5 = "";
    printf("Test 5 Input: ''\\n");
    char* out5 = process_string(test5);
    if (out5) {
        printf("Test 5 Output: '%s'\\n", out5);
        free(out5);
        out5 = NULL;
    }
    
    return 0;
}
