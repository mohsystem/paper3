
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_LENGTH 1024

char* process_string(const char* input, char* output, size_t output_size) {
    if (input == NULL || output == NULL || output_size == 0) {
        if (output != NULL && output_size > 0) {
            snprintf(output, output_size, "Error: Invalid input");
        }
        return output;
    }
    
    size_t length = strnlen(input, MAX_LENGTH + 1);
    
    if (length > MAX_LENGTH) {
        snprintf(output, output_size, "Error: Input exceeds maximum length of %d", MAX_LENGTH);
        return output;
    }
    
    // Safe buffer for trimmed string
    char safe_buffer[MAX_LENGTH + 1];
    strncpy(safe_buffer, input, MAX_LENGTH);
    safe_buffer[MAX_LENGTH] = '\\0';
    
    // Trim leading whitespace
    char* start = safe_buffer;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Trim trailing whitespace
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\\0';
        end--;
    }
    
    // Safely format output
    snprintf(output, output_size, "Processed: %s (Length: %zu)", start, length);
    
    return output;
}

int main() {
    char output[MAX_LENGTH + 100];
    
    // Test case 1: Normal input
    process_string("Hello World", output, sizeof(output));
    printf("Test 1: %s\\n", output);
    
    // Test case 2: Empty string
    process_string("", output, sizeof(output));
    printf("Test 2: %s\\n", output);
    
    // Test case 3: String with whitespace
    process_string("  Test String  ", output, sizeof(output));
    printf("Test 3: %s\\n", output);
    
    // Test case 4: Null input
    process_string(NULL, output, sizeof(output));
    printf("Test 4: %s\\n", output);
    
    // Test case 5: Long string
    char long_string[600];
    memset(long_string, 'A', 500);
    long_string[500] = '\\0';
    process_string(long_string, output, sizeof(output));
    printf("Test 5: %s\\n", output);
    
    // Interactive mode
    printf("\\nEnter a string to process (or 'exit' to quit):\\n");
    char input[MAX_LENGTH + 1];
    while (fgets(input, sizeof(input), stdin) != NULL) {
        // Remove newline
        input[strcspn(input, "\\n")] = '\\0';
        
        if (strcmp(input, "exit") == 0) {
            break;
        }
        
        process_string(input, output, sizeof(output));
        printf("%s\\n", output);
        printf("Enter another string (or 'exit' to quit):\\n");
    }
    
    return 0;
}
