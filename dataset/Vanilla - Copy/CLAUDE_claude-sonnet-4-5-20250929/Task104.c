
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

void handleUserInput(const char* input, char* buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    
    if (input == NULL) {
        return;
    }
    
    size_t length = strlen(input);
    size_t copyLength = (length < BUFFER_SIZE) ? length : BUFFER_SIZE - 1;
    
    strncpy(buffer, input, copyLength);
    buffer[copyLength] = '\\0';
}

char* handleUserInputAlloc(const char* input) {
    if (input == NULL) {
        char* result = (char*)malloc(1);
        result[0] = '\\0';
        return result;
    }
    
    size_t length = strlen(input);
    size_t copyLength = (length < BUFFER_SIZE) ? length : BUFFER_SIZE;
    
    char* result = (char*)malloc(copyLength + 1);
    strncpy(result, input, copyLength);
    result[copyLength] = '\\0';
    
    return result;
}

int main() {
    char buffer[BUFFER_SIZE];
    
    printf("Test Case 1: Normal input\\n");
    const char* input1 = "Hello, World!";
    handleUserInput(input1, buffer);
    printf("Input: %s\\n", input1);
    printf("Output: %s\\n", buffer);
    printf("Length: %zu\\n\\n", strlen(buffer));
    
    printf("Test Case 2: Empty input\\n");
    const char* input2 = "";
    handleUserInput(input2, buffer);
    printf("Input: (empty)\\n");
    printf("Output: %s\\n", buffer);
    printf("Length: %zu\\n\\n", strlen(buffer));
    
    printf("Test Case 3: Input exceeding buffer size\\n");
    char input3[151];
    memset(input3, 'A', 150);
    input3[150] = '\\0';
    handleUserInput(input3, buffer);
    printf("Input length: 150\\n");
    printf("Output length: %zu\\n", strlen(buffer));
    printf("Truncated: %s\\n\\n", strlen(buffer) == BUFFER_SIZE - 1 ? "true" : "false");
    
    printf("Test Case 4: Input at buffer boundary\\n");
    char input4[101];
    memset(input4, 'B', 100);
    input4[100] = '\\0';
    handleUserInput(input4, buffer);
    printf("Input length: 100\\n");
    printf("Output length: %zu\\n\\n", strlen(buffer));
    
    printf("Test Case 5: NULL input\\n");
    handleUserInput(NULL, buffer);
    printf("Input: NULL\\n");
    printf("Output: %s\\n", buffer);
    printf("Length: %zu\\n", strlen(buffer));
    
    return 0;
}
