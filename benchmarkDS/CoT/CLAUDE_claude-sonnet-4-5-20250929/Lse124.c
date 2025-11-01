
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void trim_trailing_whitespace(char* str) {
    if (str == NULL) return;
    
    int len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        str[len - 1] = '\\0';
        len--;
    }
}

int main(int argc, char* argv[]) {
    // Check if exactly one argument is provided
    if (argc != 2) {
        fprintf(stderr, "Error: Exactly one argument is required.\\n");
        exit(1);
    }
    
    // Get the argument from command line
    char input[1024];
    strncpy(input, argv[1], sizeof(input) - 1);
    input[sizeof(input) - 1] = '\\0';
    
    // Print the argument
    printf("Original: %s\\n", input);
    
    // Trim trailing whitespace
    trim_trailing_whitespace(input);
    
    // Print the trimmed argument
    printf("Trimmed: %s\\n", input);
    
    // Exit the program
    exit(0);
}
