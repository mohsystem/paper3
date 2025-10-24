
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_INPUT_LENGTH 1024

char* processString(const char* input, char* output, size_t outputSize) {
    // Validate input
    if (input == NULL || output == NULL || outputSize == 0) {
        if (output != NULL && outputSize > 0) {
            snprintf(output, outputSize, "Error: Invalid input");
        }
        return output;
    }
    
    size_t inputLen = strlen(input);
    
    // Check for buffer overflow
    if (inputLen > MAX_INPUT_LENGTH) {
        snprintf(output, outputSize, "Error: Input exceeds maximum length of %d", MAX_INPUT_LENGTH);
        return output;
    }
    
    // Create safe temporary buffer
    char temp[MAX_INPUT_LENGTH + 1];
    
    // Copy input safely with bounds checking
    strncpy(temp, input, MAX_INPUT_LENGTH);
    temp[MAX_INPUT_LENGTH] = '\\0';
    
    // Trim leading whitespace
    char* start = temp;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\\0';
    
    // Convert to uppercase
    for (char* p = start; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
    
    // Safely copy result to output buffer
    snprintf(output, outputSize, "Processed: %s", start);
    
    return output;
}

int main() {
    char output[MAX_INPUT_LENGTH + 50];
    
    // Test case 1: Normal string
    processString("Hello World", output, sizeof(output));
    printf("Test 1: %s\\n", output);
    
    // Test case 2: Empty string
    processString("", output, sizeof(output));
    printf("Test 2: %s\\n", output);
    
    // Test case 3: String with whitespace
    processString("  C Programming  ", output, sizeof(output));
    printf("Test 3: %s\\n", output);
    
    // Test case 4: NULL input
    processString(NULL, output, sizeof(output));
    printf("Test 4: %s\\n", output);
    
    // Test case 5: Very long string
    char longString[2500];
    memset(longString, 'A', 2000);
    longString[2000] = '\\0';
    processString(longString, output, sizeof(output));
    printf("Test 5: %s\\n", output);
    
    return 0;
}
