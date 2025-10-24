
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Maximum string length to prevent excessive memory usage: 10MB */
#define MAX_STRING_LENGTH (10 * 1024 * 1024)

/* Function to reverse a string safely
 * Parameters:
 *   input: null-terminated input string (must not be NULL)
 *   output: pre-allocated buffer for reversed string (must not be NULL)
 *   output_size: size of output buffer
 * Returns: 0 on success, -1 on error
 * Security: Validates all inputs, checks bounds, ensures null termination
 */
int reverseString(const char* input, char* output, size_t output_size) {
    size_t input_len;
    size_t i;
    
    /* Validate input pointer is not NULL */
    if (input == NULL) {
        fprintf(stderr, "Error: input string is NULL\\n");
        return -1;
    }
    
    /* Validate output pointer is not NULL */
    if (output == NULL) {
        fprintf(stderr, "Error: output buffer is NULL\\n");
        return -1;
    }
    
    /* Validate output buffer size is at least 1 (for null terminator) */
    if (output_size < 1) {
        fprintf(stderr, "Error: output buffer size is too small\\n");
        return -1;
    }
    
    /* Calculate input length with bounds check using strnlen
     * strnlen is safer than strlen as it limits the scan length
     */
    input_len = strnlen(input, MAX_STRING_LENGTH + 1);
    
    /* Validate input length is within allowed maximum */
    if (input_len > MAX_STRING_LENGTH) {
        fprintf(stderr, "Error: input string exceeds maximum length\\n");
        return -1;
    }
    
    /* Validate output buffer is large enough (need input_len + 1 for null) */
    if (output_size < input_len + 1) {
        fprintf(stderr, "Error: output buffer too small for reversed string\\n");
        return -1;
    }
    
    /* Reverse the string by copying from end to beginning
     * Bounds are checked: i < input_len ensures we don't read past input\n     * and i always writes to valid positions in output buffer\n     */\n    for (i = 0; i < input_len; i++) {\n        /* Check to prevent any potential overflow (defensive coding) */\n        if (i >= output_size - 1) {\n            fprintf(stderr, "Error: buffer overflow prevented\\n");\n            return -1;\n        }\n        output[i] = input[input_len - 1 - i];\n    }\n    \n    /* Ensure null termination of output string */\n    output[input_len] = '\\0';
    
    return 0;
}

int main(void) {
    char output[1024];
    int result;
    
    /* Initialize output buffer to prevent undefined behavior */
    memset(output, 0, sizeof(output));
    
    /* Test case 1: Normal string */
    printf("Test 1: Normal string\\n");
    result = reverseString("hello", output, sizeof(output));
    if (result == 0) {
        printf("Input: \\"hello\\" -> Output: \\"%s\\"\\n", output);
    }
    
    /* Clear buffer between tests */
    memset(output, 0, sizeof(output));
    
    /* Test case 2: Empty string */
    printf("\\nTest 2: Empty string\\n");
    result = reverseString("", output, sizeof(output));
    if (result == 0) {
        printf("Input: \\"\\" -> Output: \\"%s\\"\\n", output);
    }
    
    /* Clear buffer between tests */
    memset(output, 0, sizeof(output));
    
    /* Test case 3: Single character */
    printf("\\nTest 3: Single character\\n");
    result = reverseString("a", output, sizeof(output));
    if (result == 0) {
        printf("Input: \\"a\\" -> Output: \\"%s\\"\\n", output);
    }
    
    /* Clear buffer between tests */
    memset(output, 0, sizeof(output));
    
    /* Test case 4: String with spaces and special characters */
    printf("\\nTest 4: String with spaces and special characters\\n");
    result = reverseString("Hello, World! 123", output, sizeof(output));
    if (result == 0) {
        printf("Input: \\"Hello, World! 123\\" -> Output: \\"%s\\"\\n", output);
    }
    
    /* Clear buffer between tests */
    memset(output, 0, sizeof(output));
    
    /* Test case 5: Palindrome */
    printf("\\nTest 5: Palindrome\\n");
    result = reverseString("racecar", output, sizeof(output));
    if (result == 0) {
        printf("Input: \\"racecar\\" -> Output: \\"%s\\"\\n", output);
    }
    
    return 0;
}
