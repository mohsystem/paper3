
#include <stdio.h>
#include <string.h>
#include <ctype.h>

void processAndPrint(const char* input) {
    if (input == NULL) {
        printf("\\n");
        return;
    }
    
    int len = strlen(input);
    int end = len - 1;
    
    // Find the last non-whitespace character
    while (end >= 0 && isspace((unsigned char)input[end])) {
        end--;
    }
    
    // Print characters up to and including the last non-whitespace
    for (int i = 0; i <= end; i++) {
        putchar(input[i]);
    }
    printf("\\n");
}

int main(int argc, char* argv[]) {
    // Test case 1: Normal string with trailing spaces
    printf("Test 1:\\n");
    processAndPrint("Hello World   ");
    
    // Test case 2: String with trailing tabs and spaces
    printf("\\nTest 2:\\n");
    processAndPrint("C Programming\\t\\t  ");
    
    // Test case 3: String with no trailing whitespace
    printf("\\nTest 3:\\n");
    processAndPrint("No trailing space");
    
    // Test case 4: String with only whitespace
    printf("\\nTest 4:\\n");
    processAndPrint("    ");
    
    // Test case 5: Empty string
    printf("\\nTest 5:\\n");
    processAndPrint("");
    
    // Process command line arguments if provided
    if (argc > 1) {
        printf("\\nCommand line argument:\\n");
        processAndPrint(argv[1]);
    }
    
    return 0;
}
