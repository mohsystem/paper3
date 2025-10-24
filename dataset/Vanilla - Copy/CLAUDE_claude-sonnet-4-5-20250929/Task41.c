
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 1024

// Safe string processing function with buffer overflow protection
void processString(const char* input, char* output, size_t outputSize) {
    if (input == NULL || output == NULL || outputSize == 0) {
        return;
    }
    
    const char* prefix = "Processed: ";
    size_t prefixLen = strlen(prefix);
    size_t inputLen = strlen(input);
    
    // Clear output buffer
    memset(output, 0, outputSize);
    
    // Check if we have enough space
    if (prefixLen + inputLen + 1 > outputSize) {
        // Calculate how much of input we can safely copy
        size_t availableSpace = outputSize - prefixLen - 1;
        
        // Use strncpy for safe copying with size limit
        strncpy(output, prefix, outputSize - 1);
        strncat(output, input, availableSpace);
    } else {
        // Safe to copy everything
        strncpy(output, prefix, outputSize - 1);
        strncat(output, input, outputSize - strlen(output) - 1);
    }
    
    // Ensure null termination
    output[outputSize - 1] = '\\0';
}

int main() {
    char buffer[MAX_BUFFER_SIZE];
    
    // Test case 1: Normal string
    processString("Hello World", buffer, sizeof(buffer));
    printf("Test 1: %s\\n", buffer);
    
    // Test case 2: Empty string
    processString("", buffer, sizeof(buffer));
    printf("Test 2: %s\\n", buffer);
    
    // Test case 3: Very long string that would overflow
    char longInput[2000];
    memset(longInput, 'a', sizeof(longInput) - 1);
    longInput[sizeof(longInput) - 1] = '\\0';
    processString(longInput, buffer, sizeof(buffer));
    printf("Test 3: %.50s...\\n", buffer);
    
    // Test case 4: Special characters
    processString("!@#$%^&*()_+-=[]{}|;':,.<>?", buffer, sizeof(buffer));
    printf("Test 4: %s\\n", buffer);
    
    // Test case 5: Small buffer size
    char smallBuffer[30];
    processString("This is a long test string", smallBuffer, sizeof(smallBuffer));
    printf("Test 5: %s\\n", smallBuffer);
    
    return 0;
}
