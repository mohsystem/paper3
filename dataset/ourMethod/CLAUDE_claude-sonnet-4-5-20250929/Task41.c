
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Maximum input length to prevent excessive memory usage and DoS
#define MAX_INPUT_LENGTH (1024 * 1024)
// Initial buffer size for reading input
#define INITIAL_BUFFER_SIZE 256

// Safely read a line from stdin with dynamic buffer growth
// Returns allocated string that caller must free, or NULL on error
char* safeReadLine(void) {
    size_t bufferSize = INITIAL_BUFFER_SIZE;
    size_t position = 0;
    
    // Allocate initial buffer - checked for NULL
    char* buffer = (char*)malloc(bufferSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Read character by character to maintain control
    int c;
    while ((c = getchar()) != EOF && c != '\\n') {
        // Check if we need to grow the buffer
        if (position >= bufferSize - 1) {
            // Prevent buffer from growing beyond maximum
            if (bufferSize >= MAX_INPUT_LENGTH) {
                fprintf(stderr, "Error: Input exceeds maximum length\\n");
                free(buffer);
                return NULL;
            }
            
            // Double buffer size with overflow check
            size_t newSize = bufferSize * 2;
            if (newSize > MAX_INPUT_LENGTH) {
                newSize = MAX_INPUT_LENGTH;
            }
            if (newSize <= bufferSize) {
                // Overflow detected
                fprintf(stderr, "Error: Buffer size overflow\\n");
                free(buffer);
                return NULL;
            }
            
            // Reallocate buffer - checked for NULL
            char* newBuffer = (char*)realloc(buffer, newSize);
            if (newBuffer == NULL) {
                fprintf(stderr, "Error: Memory reallocation failed\\n");
                free(buffer);
                return NULL;
            }
            buffer = newBuffer;
            bufferSize = newSize;
        }
        
        // Validate character is in valid ASCII range
        if (c >= 0 && c <= 255) {
            buffer[position++] = (char)c;
        }
    }
    
    // Null-terminate the string - bounds already checked above
    buffer[position] = '\\0';
    
    return buffer;
}

// Process string safely with bounds checking
// Returns newly allocated processed string or NULL on error
char* processString(const char* input) {
    // Validate input pointer
    if (input == NULL) {
        fprintf(stderr, "Error: NULL input\\n");
        return NULL;
    }
    
    // Validate input length
    size_t inputLen = strlen(input);
    if (inputLen == 0) {
        char* result = (char*)malloc(20);
        if (result == NULL) {
            return NULL;
        }
        snprintf(result, 20, "Error: Empty input");
        return result;
    }
    
    if (inputLen > MAX_INPUT_LENGTH) {
        char* result = (char*)malloc(50);
        if (result == NULL) {
            return NULL;
        }
        snprintf(result, 50, "Error: Input exceeds maximum length");
        return result;
    }
    
    // Allocate output buffer with space for prefix and null terminator
    // Size calculation with overflow check
    size_t prefixLen = 50; // Space for "Processed (X chars): "
    size_t totalSize = prefixLen + inputLen + 1;
    
    if (totalSize < inputLen) {
        // Overflow detected
        fprintf(stderr, "Error: Size calculation overflow\\n");
        return NULL;
    }
    
    char* output = (char*)malloc(totalSize);
    if (output == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    // Initialize output buffer
    memset(output, 0, totalSize);
    
    // Create prefix with bounds-checked formatting
    int written = snprintf(output, totalSize, "Processed (%zu chars): ", inputLen);
    if (written < 0 || (size_t)written >= totalSize) {
        free(output);
        return NULL;
    }
    
    // Process input string safely with bounds checking
    size_t outPos = (size_t)written;
    for (size_t i = 0; i < inputLen && input[i] != '\\0'; i++) {
        // Bounds check before writing
        if (outPos >= totalSize - 1) {
            break;
        }
        
        unsigned char ch = (unsigned char)input[i];
        // Validate character is printable or whitespace
        if ((ch >= 32 && ch <= 126) || ch == '\\n' || ch == '\\t') {
            output[outPos++] = (char)toupper(ch);
        } else {
            // Replace non-printable with safe character
            output[outPos++] = '?';
        }
    }
    
    // Ensure null termination
    output[outPos] = '\\0';
    
    return output;
}

int main(void) {
    // Test case 1: Normal string
    const char* test1 = "Hello World";
    char* result1 = processString(test1);
    if (result1 != NULL) {
        printf("Test 1: %s\\n", result1);
        free(result1);
    }
    
    // Test case 2: Empty string
    const char* test2 = "";
    char* result2 = processString(test2);
    if (result2 != NULL) {
        printf("Test 2: %s\\n", result2);
        free(result2);
    }
    
    // Test case 3: String with special characters
    const char* test3 = "Test@123!";
    char* result3 = processString(test3);
    if (result3 != NULL) {
        printf("Test 3: %s\\n", result3);
        free(result3);
    }
    
    // Test case 4: Long string
    char* test4 = (char*)malloc(501);
    if (test4 != NULL) {
        memset(test4, 'a', 500);
        test4[500] = '\\0';
        char* result4 = processString(test4);
        if (result4 != NULL) {
            printf("Test 4: %.50s...\\n", result4);
            free(result4);
        }
        free(test4);
    }
    
    // Test case 5: String with whitespace
    const char* test5 = "Hello\\tWorld\\nTest";
    char* result5 = processString(test5);
    if (result5 != NULL) {
        printf("Test 5: %s\\n", result5);
        free(result5);
    }
    
    // Interactive mode with safe input reading
    printf("\\nEnter a string to process (or 'quit' to exit): ");
    
    char* userInput = NULL;
    while ((userInput = safeReadLine()) != NULL) {
        if (strcmp(userInput, "quit") == 0) {
            free(userInput);
            break;
        }
        
        char* processed = processString(userInput);
        if (processed != NULL) {
            printf("%s\\n", processed);
            free(processed);
        }
        
        free(userInput);
        printf("\\nEnter another string (or 'quit' to exit): ");
    }
    
    return 0;
}
