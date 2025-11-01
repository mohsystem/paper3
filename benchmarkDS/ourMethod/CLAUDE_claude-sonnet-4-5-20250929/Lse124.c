
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum input length to prevent resource exhaustion
#define MAX_INPUT_LENGTH 10240

// Function to trim trailing whitespace in place
void trimTrailing(char* str) {
    if (str == NULL || *str == '\\0') {
        return;
    }
    
    size_t len = strlen(str);
    // Work backwards from end to find last non-whitespace character
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\\t' || 
                       str[len - 1] == '\\n' || str[len - 1] == '\\r' ||
                       str[len - 1] == '\\f' || str[len - 1] == '\\v')) {
        len--;
    }
    // Null terminate at the new end position
    str[len] = '\\0';
}

void processArgument(int argc, char* argv[]) {
    // First if statement: check that exactly one argument is provided
    // argc includes program name, so we check for argc == 2
    if (argc != 2) {
        fprintf(stderr, "Error: exactly one argument required\\n");
        exit(1);
    }
    
    // Second line: get the argument from command line and store in variable called input
    // Input validation: check for null pointer
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: invalid argument\\n");
        exit(1);
    }
    
    size_t argLen = strlen(argv[1]);
    // Limit input size to prevent memory exhaustion
    if (argLen > MAX_INPUT_LENGTH) {
        fprintf(stderr, "Error: argument too long\\n");
        exit(1);
    }
    
    // Allocate buffer with extra space for null terminator
    // Check allocation result
    char* input = (char*)malloc(argLen + 1);
    if (input == NULL) {
        fprintf(stderr, "Error: memory allocation failed\\n");
        exit(1);
    }
    
    // Safe copy with bounds checking - use strncpy and ensure null termination
    strncpy(input, argv[1], argLen);
    input[argLen] = '\\0';
    
    // Third line: print the argument
    // Use puts for safe output or printf with format string
    printf("%s\\n", input);
    
    // Fourth line: trim trailing whitespace from the argument
    trimTrailing(input);
    
    // Fifth line: print the argument again
    printf("%s\\n", input);
    
    // Free allocated memory before exit
    free(input);
    input = NULL;
    
    // Sixth line: exit the program
    exit(0);
}

int main(int argc, char* argv[]) {
    // Test cases - uncomment one at a time to test
    // Test 1: Normal input
    // char* test1[] = {"program", "hello", NULL};
    // processArgument(2, test1);
    
    // Test 2: Input with trailing spaces
    // char* test2[] = {"program", "hello   ", NULL};
    // processArgument(2, test2);
    
    // Test 3: Input with trailing tabs
    // char* test3[] = {"program", "world\\t\\t", NULL};
    // processArgument(2, test3);
    
    // Test 4: Input with mixed trailing whitespace
    // char* test4[] = {"program", "test \\t \\n", NULL};
    // processArgument(2, test4);
    
    // Test 5: Empty string
    // char* test5[] = {"program", "", NULL};
    // processArgument(2, test5);
    
    // Run with actual command line arguments
    processArgument(argc, argv);
    
    return 0;
}
