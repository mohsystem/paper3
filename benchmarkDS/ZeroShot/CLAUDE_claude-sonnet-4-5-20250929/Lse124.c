
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void processInput(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        fprintf(stderr, "Error: Exactly one argument required\\n");
        exit(1);
    }
    
    // Get the argument from command line
    char* input = argv[1];
    
    // Print the argument
    printf("Original input: %s\\n", input);
    
    // Trim trailing whitespace
    int len = strlen(input);
    while (len > 0 && (input[len-1] == ' ' || input[len-1] == '\\t' || 
                       input[len-1] == '\\n' || input[len-1] == '\\r')) {
        input[len-1] = '\\0';
        len--;
    }
    
    // Print the argument again
    printf("Trimmed input: %s\\n", input);
}

int main() {
    // Test cases
    char* testCases[][2] = {
        {"prog", "Hello"},
        {"prog", "World   "},
        {"prog", "Test\\t\\t"},
        {"prog", "NoWhitespace"},
        {"prog", "Multiple Words  "}
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d:\\n", i + 1);
        char input_copy[100];
        strcpy(input_copy, testCases[i][1]);
        char* argv[2] = {testCases[i][0], input_copy};
        processInput(2, argv);
        printf("\\n");
    }
    
    return 0;
}
