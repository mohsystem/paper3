
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

/**
 * Safely handles user input into a fixed-size buffer
 * @param input The input string to be stored
 * @param output Buffer to store the result
 * @return Length of stored string
 */
int handleInput(const char* input, char* output) {
    if (input == NULL || output == NULL) {
        if (output != NULL) {
            output[0] = '\\0';
        }
        return 0;
    }
    
    size_t input_len = strlen(input);
    size_t copy_len = (input_len < BUFFER_SIZE - 1) ? input_len : BUFFER_SIZE - 1;
    
    strncpy(output, input, copy_len);
    output[copy_len] = '\\0'; // Ensure null termination
    
    return copy_len;
}

/**
 * Stores input into a character buffer safely
 * @param input The input string
 * @param buffer Pre-allocated buffer to store the input
 */
void storeInBuffer(const char* input, char* buffer) {
    if (buffer == NULL) {
        return;
    }
    
    // Initialize buffer with null characters
    memset(buffer, 0, BUFFER_SIZE);
    
    if (input == NULL || strlen(input) == 0) {
        return;
    }
    
    size_t length = strlen(input);
    if (length > BUFFER_SIZE - 1) {
        length = BUFFER_SIZE - 1;
    }
    
    strncpy(buffer, input, length);
    buffer[length] = '\\0'; // Ensure null termination
}

int main() {
    printf("Testing Fixed-Size Buffer Input Handler\\n\\n");
    
    // Test case 1: Normal input
    char test1[] = "Hello, World!";
    char output1[BUFFER_SIZE];
    char buffer1[BUFFER_SIZE];
    printf("Test 1 - Normal input:\\n");
    printf("Input: %s\\n", test1);
    handleInput(test1, output1);
    printf("Output: %s\\n", output1);
    storeInBuffer(test1, buffer1);
    printf("Buffer: %s\\n\\n", buffer1);
    
    // Test case 2: Empty input
    char test2[] = "";
    char output2[BUFFER_SIZE];
    printf("Test 2 - Empty input:\\n");
    printf("Input: \\"\\"\\n");
    handleInput(test2, output2);
    printf("Output: %s\\n\\n", output2);
    
    // Test case 3: Input exceeding buffer size
    char test3[151];
    memset(test3, 'A', 150);
    test3[150] = '\\0';
    char output3[BUFFER_SIZE];
    printf("Test 3 - Input exceeding buffer size (150 chars):\\n");
    printf("Input length: %lu\\n", strlen(test3));
    int result3_len = handleInput(test3, output3);
    printf("Output length: %d\\n", result3_len);
    printf("Truncated: %s\\n\\n", (strlen(test3) > result3_len) ? "true" : "false");
    
    // Test case 4: Input at buffer boundary
    char test4[101];
    memset(test4, 'B', 100);
    test4[100] = '\\0';
    char output4[BUFFER_SIZE];
    printf("Test 4 - Input at buffer boundary (100 chars):\\n");
    printf("Input length: %lu\\n", strlen(test4));
    int result4_len = handleInput(test4, output4);
    printf("Output length: %d\\n\\n", result4_len);
    
    // Test case 5: Null input
    char* test5 = NULL;
    char output5[BUFFER_SIZE];
    printf("Test 5 - Null input:\\n");
    printf("Input: NULL\\n");
    handleInput(test5, output5);
    printf("Output: \\"%s\\"\\n", output5);
    
    return 0;
}
