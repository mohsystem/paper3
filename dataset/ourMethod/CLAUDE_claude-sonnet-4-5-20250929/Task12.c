
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

// Security: Maximum lengths to prevent buffer overflow and DoS
#define MAX_INPUT_LENGTH 1000000
#define MAX_WORD_LENGTH 1000
#define MAX_WORDS 10000

// Security: Validate character is lowercase letter
bool isValidLowercase(char c) {
    return c >= 'a' && c <= 'z';
}

// Security: Safe score calculation with overflow protection
bool calculateWordScore(const char* word, size_t wordLen, uint32_t* score) {
    // Security: Validate pointers are not NULL
    if (word == NULL || score == NULL) {
        return false;
    }
    
    // Security: Validate word length
    if (wordLen == 0) {
        *score = 0;
        return true;
    }
    
    // Security: Check maximum possible score won't overflow\n    if (wordLen > UINT32_MAX / 26) {\n        return false;\n    }\n    \n    *score = 0;\n    \n    // Security: Bounds-checked loop with length validation\n    for (size_t i = 0; i < wordLen; ++i) {\n        // Security: Validate each character is lowercase letter\n        if (!isValidLowercase(word[i])) {\n            return false;\n        }\n        \n        uint32_t charScore = (uint32_t)(word[i] - 'a' + 1);\n        \n        // Security: Check for overflow before addition\n        if (*score > UINT32_MAX - charScore) {\n            return false;\n        }\n        \n        *score += charScore;\n    }\n    \n    return true;\n}\n\n// Security: Safe string copy with bounds checking\nbool safeCopy(char* dest, size_t destSize, const char* src, size_t srcLen) {\n    // Security: Validate pointers and sizes\n    if (dest == NULL || src == NULL || destSize == 0) {\n        return false;\n    }\n    \n    // Security: Ensure destination has room for null terminator\n    if (srcLen >= destSize) {\n        return false;\n    }\n    \n    // Security: Use memcpy with validated length\n    memcpy(dest, src, srcLen);\n    dest[srcLen] = '\\0'; // Ensure null termination\n    \n    return true;\n}\n\nchar* highestScoringWord(const char* input) {\n    // Security: Validate input pointer\n    if (input == NULL) {\n        char* result = (char*)malloc(1);\n        if (result != NULL) {\n            result[0] = '\\0';\n        }\n        return result;\n    }\n    \n    size_t inputLen = strlen(input);\n    \n    // Security: Validate input length to prevent DoS\n    if (inputLen > MAX_INPUT_LENGTH) {\n        char* result = (char*)malloc(1);\n        if (result != NULL) {\n            result[0] = '\\0';\n        }\n        return result;\n    }\n    \n    // Security: Allocate result buffer\n    char* highestWord = (char*)malloc(MAX_WORD_LENGTH + 1);\n    if (highestWord == NULL) {\n        return NULL; // Memory allocation failed\n    }\n    highestWord[0] = '\\0'; // Initialize to empty string\n    \n    uint32_t highestScore = 0;\n    bool foundValid = false;\n    \n    size_t wordStart = 0;\n    size_t i = 0;\n    \n    // Security: Bounds-checked parsing loop\n    while (i <= inputLen) {\n        // Word boundary: space or end of string\n        if (i == inputLen || input[i] == ' ') {
            size_t wordLen = i - wordStart;
            
            // Security: Validate word length
            if (wordLen > 0 && wordLen <= MAX_WORD_LENGTH) {
                uint32_t currentScore = 0;
                
                // Security: Safe score calculation with validation
                if (calculateWordScore(&input[wordStart], wordLen, &currentScore)) {
                    // First valid word or higher score found
                    if (!foundValid || currentScore > highestScore) {
                        // Security: Safe copy with bounds checking
                        if (safeCopy(highestWord, MAX_WORD_LENGTH + 1, 
                                   &input[wordStart], wordLen)) {
                            highestScore = currentScore;
                            foundValid = true;
                        }
                    }
                }
            }
            
            // Move to next word
            wordStart = i + 1;
        }
        
        ++i;
    }
    
    return highestWord;
}

int main(void) {
    // Test case 1: Basic example
    char* result1 = highestScoringWord("man i need a taxi up to ubud");
    printf("Test 1: \\"%s\\"\\n", result1 ? result1 : "NULL");
    free(result1); // Security: Free allocated memory
    
    // Test case 2: Single word
    char* result2 = highestScoringWord("hello");
    printf("Test 2: \\"%s\\"\\n", result2 ? result2 : "NULL");
    free(result2);
    
    // Test case 3: Same score, return first
    char* result3 = highestScoringWord("aa b");
    printf("Test 3: \\"%s\\"\\n", result3 ? result3 : "NULL");
    free(result3);
    
    // Test case 4: Different scores
    char* result4 = highestScoringWord("a bb ccc");
    printf("Test 4: \\"%s\\"\\n", result4 ? result4 : "NULL");
    free(result4);
    
    // Test case 5: Empty string edge case
    char* result5 = highestScoringWord("");
    printf("Test 5: \\"%s\\"\\n", result5 ? result5 : "NULL");
    free(result5);
    
    return 0;
}
