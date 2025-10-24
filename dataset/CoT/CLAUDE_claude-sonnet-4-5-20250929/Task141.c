
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Reverses a given string safely
 * @param input the string to reverse
 * @param output buffer to store the reversed string
 * @param output_size size of the output buffer
 * @return 0 on success, -1 on error
 */
int reverseString(const char* input, char* output, size_t output_size) {
    // Security: Validate input parameters
    if (input == NULL || output == NULL || output_size == 0) {
        return -1;
    }
    
    size_t len = strlen(input);
    
    // Security: Check buffer overflow
    if (len >= output_size) {
        return -1;
    }
    
    // Reverse the string
    for (size_t i = 0; i < len; i++) {
        output[i] = input[len - 1 - i];
    }
    
    // Security: Null-terminate the string
    output[len] = '\\0';
    
    return 0;
}

int main() {
    char output[256]; // Buffer for reversed string
    
    // Test Case 1
    printf("Test Case 1:\\n");
    const char* test1 = "hello";
    printf("Input: %s\\n", test1);
    if (reverseString(test1, output, sizeof(output)) == 0) {
        printf("Output: %s\\n", output);
    }
    printf("\\n");
    
    // Test Case 2
    printf("Test Case 2:\\n");
    const char* test2 = "Java Programming";
    printf("Input: %s\\n", test2);
    if (reverseString(test2, output, sizeof(output)) == 0) {
        printf("Output: %s\\n", output);
    }
    printf("\\n");
    
    // Test Case 3
    printf("Test Case 3:\\n");
    const char* test3 = "12345";
    printf("Input: %s\\n", test3);
    if (reverseString(test3, output, sizeof(output)) == 0) {
        printf("Output: %s\\n", output);
    }
    printf("\\n");
    
    // Test Case 4
    printf("Test Case 4:\\n");
    const char* test4 = "";
    printf("Input: (empty string)\\n");
    if (reverseString(test4, output, sizeof(output)) == 0) {
        printf("Output: %s\\n", output);
    }
    printf("\\n");
    
    // Test Case 5
    printf("Test Case 5:\\n");
    const char* test5 = "A";
    printf("Input: %s\\n", test5);
    if (reverseString(test5, output, sizeof(output)) == 0) {
        printf("Output: %s\\n", output);
    }
    
    return 0;
}
