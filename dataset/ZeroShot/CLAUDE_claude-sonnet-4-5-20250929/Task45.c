
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_INPUT_SIZE 1024
#define MAX_OUTPUT_SIZE 2048

void trim(char* str) {
    int start = 0, end = strlen(str) - 1;
    while (start <= end && isspace(str[start])) start++;
    while (end >= start && isspace(str[end])) end--;
    
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\\0';
}

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

void performOperation(const char* input, const char* operation, char* output) {
    if (input == NULL || operation == NULL) {
        snprintf(output, MAX_OUTPUT_SIZE, "Error: Input and operation cannot be NULL");
        return;
    }
    
    char inputCopy[MAX_INPUT_SIZE];
    char opCopy[MAX_INPUT_SIZE];
    
    strncpy(inputCopy, input, MAX_INPUT_SIZE - 1);
    inputCopy[MAX_INPUT_SIZE - 1] = '\\0';
    strncpy(opCopy, operation, MAX_INPUT_SIZE - 1);
    opCopy[MAX_INPUT_SIZE - 1] = '\\0';
    
    trim(inputCopy);
    trim(opCopy);
    toLowerCase(opCopy);
    
    if (strcmp(opCopy, "uppercase") == 0) {
        strcpy(output, inputCopy);
        for (int i = 0; output[i]; i++) {
            output[i] = toupper(output[i]);
        }
    }
    else if (strcmp(opCopy, "lowercase") == 0) {
        strcpy(output, inputCopy);
        for (int i = 0; output[i]; i++) {
            output[i] = tolower(output[i]);
        }
    }
    else if (strcmp(opCopy, "reverse") == 0) {
        int len = strlen(inputCopy);
        for (int i = 0; i < len; i++) {
            output[i] = inputCopy[len - 1 - i];
        }
        output[len] = '\\0';
    }
    else if (strcmp(opCopy, "length") == 0) {
        snprintf(output, MAX_OUTPUT_SIZE, "%lu", (unsigned long)strlen(inputCopy));
    }
    else if (strcmp(opCopy, "wordcount") == 0) {
        if (strlen(inputCopy) == 0) {
            strcpy(output, "0");
            return;
        }
        int count = 0;
        int inWord = 0;
        for (int i = 0; inputCopy[i]; i++) {
            if (!isspace(inputCopy[i]) && !inWord) {
                count++;
                inWord = 1;
            } else if (isspace(inputCopy[i])) {
                inWord = 0;
            }
        }
        snprintf(output, MAX_OUTPUT_SIZE, "%d", count);
    }
    else if (strcmp(opCopy, "number") == 0) {
        char* endptr;
        double num = strtod(inputCopy, &endptr);
        if (*endptr != '\\0') {
            snprintf(output, MAX_OUTPUT_SIZE, "Error: Input is not a valid number for 'number' operation");
        } else {
            snprintf(output, MAX_OUTPUT_SIZE, "%g", num * 2);
        }
    }
    else {
        snprintf(output, MAX_OUTPUT_SIZE, "Error: Unknown operation: %s", opCopy);
    }
}

int main() {
    char output[MAX_OUTPUT_SIZE];
    
    printf("=== Test Cases ===\\n\\n");
    
    // Test case 1: Uppercase operation
    performOperation("hello world", "uppercase", output);
    printf("Test 1 - Uppercase: %s\\n", output);
    
    // Test case 2: Reverse operation
    performOperation("Java Programming", "reverse", output);
    printf("Test 2 - Reverse: %s\\n", output);
    
    // Test case 3: Word count operation
    performOperation("This is a test sentence", "wordcount", output);
    printf("Test 3 - Word Count: %s\\n", output);
    
    // Test case 4: Number operation with valid input
    performOperation("42.5", "number", output);
    printf("Test 4 - Number Operation: %s\\n", output);
    
    // Test case 5: Error handling - invalid operation
    performOperation("test", "invalid_op", output);
    printf("Test 5 - Invalid Operation: %s\\n", output);
    
    return 0;
}
