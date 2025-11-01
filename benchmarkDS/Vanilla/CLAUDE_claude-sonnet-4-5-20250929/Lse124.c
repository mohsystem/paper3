
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void processInput(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        printf("Error: Please provide exactly one argument\\n");
        exit(1);
    }
    
    // Get the argument from command line
    char input[1000];
    strcpy(input, argv[1]);
    
    // Print the argument
    printf("Original input: %s\\n", input);
    
    // Trim the trailing whitespace
    int len = strlen(input);
    while (len > 0 && isspace((unsigned char)input[len - 1])) {
        input[len - 1] = '\\0';
        len--;
    }
    
    // Print the argument again
    printf("Trimmed input: %s\\n", input);
    
    // Exit the program
    exit(0);
}

int main() {
    // Test case 1: Normal string with trailing spaces
    printf("Test Case 1:\\n");
    char* args1[] = {"program", "Hello World   "};
    processInput(2, args1);
    
    printf("\\nTest Case 2:\\n");
    char* args2[] = {"program", "Test123\\t\\t"};
    processInput(2, args2);
    
    printf("\\nTest Case 3:\\n");
    char* args3[] = {"program", "NoTrailingSpace"};
    processInput(2, args3);
    
    printf("\\nTest Case 4:\\n");
    char* args4[] = {"program", "Multiple   Spaces   "};
    processInput(2, args4);
    
    printf("\\nTest Case 5:\\n");
    char* args5[] = {"program", "   Leading and trailing   "};
    processInput(2, args5);
    
    return 0;
}
