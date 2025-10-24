
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Security: Maximum input length to prevent excessive memory allocation
#define MAX_INPUT_LENGTH 10000

// Helper function to reverse a string in place with bounds checking
// Security: Uses explicit length parameter to prevent buffer overruns
static void reverseString(char* str, size_t len) {
    if (str == NULL || len == 0) {
        return;
    }
    
    size_t start = 0;
    size_t end = len - 1;
    
    // Bounds-safe swap with checked indices
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

// Main function to reverse words with 5+ letters
// Security: Returns dynamically allocated string - caller must free
// All memory operations are bounds-checked
char* spinWords(const char* sentence) {
    // Input validation: check for NULL pointer
    if (sentence == NULL) {
        return NULL;
    }
    
    size_t inputLen = strlen(sentence);
    
    // Security: Validate input length to prevent excessive allocation
    if (inputLen > MAX_INPUT_LENGTH) {
        return NULL;
    }
    
    // Validate input contains only letters and spaces
    // Security measure: prevents injection of control characters
    for (size_t i = 0; i < inputLen; i++) {
        unsigned char c = (unsigned char)sentence[i];
        if (!isalpha(c) && c != ' ') {
            return NULL;
        }
    }
    
    // Allocate result buffer - size checked above
    // Security: Add 1 for null terminator, checked malloc return
    char* result = (char*)malloc(inputLen + 1);
    if (result == NULL) {
        return NULL;
    }
    
    // Initialize all memory to prevent info leaks
    memset(result, 0, inputLen + 1);
    
    // Temporary buffer for current word with safe maximum size
    char currentWord[256];
    memset(currentWord, 0, sizeof(currentWord));
    size_t wordLen = 0;
    size_t resultPos = 0;
    
    // Process each character with explicit bounds checking
    for (size_t i = 0; i < inputLen; i++) {
        char c = sentence[i];
        
        if (c == ' ') {
            // End of word - process it
            if (wordLen >= 5) {
                // Reverse words with 5+ letters using safe helper
                reverseString(currentWord, wordLen);
            }
            
            // Copy word to result with bounds check
            if (resultPos + wordLen < inputLen + 1) {
                memcpy(result + resultPos, currentWord, wordLen);
                resultPos += wordLen;
                
                // Add space with bounds check
                if (resultPos < inputLen + 1) {
                    result[resultPos++] = ' ';
                }
            }
            
            // Clear word buffer securely
            memset(currentWord, 0, wordLen);
            wordLen = 0;
        } else {
            // Build current word with bounds check
            if (wordLen < sizeof(currentWord) - 1) {
                currentWord[wordLen++] = c;
            }
        }
    }
    
    // Process last word with bounds checking
    if (wordLen >= 5) {
        reverseString(currentWord, wordLen);
    }
    
    if (resultPos + wordLen < inputLen + 1) {
        memcpy(result + resultPos, currentWord, wordLen);
        resultPos += wordLen;
    }
    
    // Ensure null termination
    result[resultPos] = '\\0';
    
    // Clear sensitive temporary buffer
    memset(currentWord, 0, sizeof(currentWord));
    
    return result;
}

int main(void) {
    // Test cases - all allocated strings must be freed
    char* result1 = spinWords("Hey fellow warriors");
    if (result1 != NULL) {
        printf("Test 1: \\"%s\\"\\n", result1);
        free(result1);
        result1 = NULL;
    }
    
    char* result2 = spinWords("This is a test");
    if (result2 != NULL) {
        printf("Test 2: \\"%s\\"\\n", result2);
        free(result2);
        result2 = NULL;
    }
    
    char* result3 = spinWords("This is another test");
    if (result3 != NULL) {
        printf("Test 3: \\"%s\\"\\n", result3);
        free(result3);
        result3 = NULL;
    }
    
    char* result4 = spinWords("Welcome");
    if (result4 != NULL) {
        printf("Test 4: \\"%s\\"\\n", result4);
        free(result4);
        result4 = NULL;
    }
    
    char* result5 = spinWords("a");
    if (result5 != NULL) {
        printf("Test 5: \\"%s\\"\\n", result5);
        free(result5);
        result5 = NULL;
    }
    
    return 0;
}
