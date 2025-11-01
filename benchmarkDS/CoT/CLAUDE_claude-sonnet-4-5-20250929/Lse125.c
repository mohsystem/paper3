
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void processArgument(const char* arg, char* result, size_t resultSize) {
    if (arg == NULL || result == NULL || resultSize == 0) {
        if (result != NULL && resultSize > 0) {
            result[0] = '\\0';
        }
        return;
    }
    
    size_t len = strlen(arg);
    if (len >= resultSize) {
        len = resultSize - 1;
    }
    
    strncpy(result, arg, len);
    result[len] = '\\0';
    
    // Trim trailing whitespace
    while (len > 0 && isspace((unsigned char)result[len - 1])) {
        len--;
        result[len] = '\\0';
    }
}

int main(int argc, char* argv[]) {
    char buffer[1024];
    
    // Test cases
    const char* testCases[] = {
        "Hello World   ",
        "NoSpaces",
        "Trailing\\t\\t",
        "   LeadingAndTrailing   ",
        ""
    };
    
    printf("Test cases:\\n");
    for (int i = 0; i < 5; i++) {
        processArgument(testCases[i], buffer, sizeof(buffer));
        printf("Test %d: [%s]\\n", i + 1, buffer);
    }
    
    // Process command line argument if provided
    if (argc > 1) {
        processArgument(argv[1], buffer, sizeof(buffer));
        printf("\\nCommand line argument: [%s]\\n", buffer);
    } else {
        printf("\\nNo command line argument provided\\n");
    }
    
    return 0;
}
