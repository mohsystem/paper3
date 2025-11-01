
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
    
    // Print characters up to the last non-whitespace character
    for (int i = 0; i <= end; i++) {
        putchar(input[i]);
    }
    printf("\\n");
}

int main(int argc, char* argv[]) {
    // Test case 1: String with trailing spaces
    printf("Test 1:\\n");
    processAndPrint("Hello World   ");
    
    // Test case 2: String with trailing tabs
    printf("\\nTest 2:\\n");
    processAndPrint("Hello World\\t\\t");
    
    // Test case 3: String with mixed trailing whitespace
    printf("\\nTest 3:\\n");
    processAndPrint("Hello World \\t \\n");
    
    // Test case 4: String with no trailing whitespace
    printf("\\nTest 4:\\n");
    processAndPrint("Hello World");
    
    // Test case 5: Empty string
    printf("\\nTest 5:\\n");
    processAndPrint("");
    
    // If command line arguments are provided
    if (argc > 1) {
        printf("\\nCommand line argument:\\n");
        processAndPrint(argv[1]);
    }
    
    return 0;
}
