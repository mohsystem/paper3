
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Copy string safely with bounds checking
char* copyString(const char* source) {
    if (source == NULL) {
        return NULL;
    }
    
    size_t len = strlen(source);
    char* dest = (char*)malloc((len + 1) * sizeof(char));
    if (dest == NULL) {
        return NULL;
    }
    
    strncpy(dest, source, len);
    dest[len] = '\\0';
    return dest;
}

// Concatenate strings safely with bounds checking
char* concatenateStrings(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\\0';
        return empty;
    }
    
    if (str1 == NULL) return copyString(str2);
    if (str2 == NULL) return copyString(str1);
    
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = (char*)malloc((len1 + len2 + 1) * sizeof(char));
    
    if (result == NULL) {
        return NULL;
    }
    
    strncpy(result, str1, len1);
    strncpy(result + len1, str2, len2);
    result[len1 + len2] = '\\0';
    
    return result;
}

// Reverse string safely
char* reverseString(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    char* reversed = (char*)malloc((len + 1) * sizeof(char));
    
    if (reversed == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        reversed[i] = input[len - 1 - i];
    }
    reversed[len] = '\\0';
    
    return reversed;
}

// Convert to uppercase safely
char* toUpperCaseSafe(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    char* upper = (char*)malloc((len + 1) * sizeof(char));
    
    if (upper == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        upper[i] = toupper((unsigned char)input[i]);
    }
    upper[len] = '\\0';
    
    return upper;
}

// Convert to lowercase safely
char* toLowerCaseSafe(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    char* lower = (char*)malloc((len + 1) * sizeof(char));
    
    if (lower == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        lower[i] = tolower((unsigned char)input[i]);
    }
    lower[len] = '\\0';
    
    return lower;
}

// Get substring with bounds checking
char* getSubstringSafe(const char* input, size_t start, size_t end) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    if (start > len || end > len || start > end) {
        return NULL;
    }
    
    size_t subLen = end - start;
    char* substring = (char*)malloc((subLen + 1) * sizeof(char));
    
    if (substring == NULL) {
        return NULL;
    }
    
    strncpy(substring, input + start, subLen);
    substring[subLen] = '\\0';
    
    return substring;
}

// Replace substring safely
char* replaceSubstring(const char* input, const char* target, const char* replacement) {
    if (input == NULL || target == NULL || replacement == NULL || strlen(target) == 0) {
        return copyString(input);
    }
    
    size_t inputLen = strlen(input);
    size_t targetLen = strlen(target);
    size_t replacementLen = strlen(replacement);
    
    // Count occurrences
    size_t count = 0;
    const char* temp = input;
    while ((temp = strstr(temp, target)) != NULL) {
        count++;
        temp += targetLen;
    }
    
    // Calculate result size
    size_t resultLen = inputLen + count * (replacementLen - targetLen);
    char* result = (char*)malloc((resultLen + 1) * sizeof(char));
    
    if (result == NULL) {
        return NULL;
    }
    
    char* resultPtr = result;
    const char* inputPtr = input;
    
    while (*inputPtr != '\\0') {
        const char* match = strstr(inputPtr, target);
        if (match == inputPtr) {
            strncpy(resultPtr, replacement, replacementLen);
            resultPtr += replacementLen;
            inputPtr += targetLen;
        } else {
            *resultPtr++ = *inputPtr++;
        }
    }
    *resultPtr = '\\0';
    
    return result;
}

int main() {
    printf("=== String Operations Test Cases ===\\n\\n");
    
    // Test Case 1: Copy String
    printf("Test Case 1 - Copy String:\\n");
    const char* original = "Hello, World!";
    char* copied = copyString(original);
    printf("Original: %s\\n", original);
    printf("Copied: %s\\n", copied);
    printf("\\n");
    free(copied);
    
    // Test Case 2: Concatenate Strings
    printf("Test Case 2 - Concatenate Strings:\\n");
    const char* str1 = "Hello";
    const char* str2 = " C";
    char* concatenated = concatenateStrings(str1, str2);
    printf("String 1: %s\\n", str1);
    printf("String 2: %s\\n", str2);
    printf("Concatenated: %s\\n", concatenated);
    printf("\\n");
    free(concatenated);
    
    // Test Case 3: Reverse String
    printf("Test Case 3 - Reverse String:\\n");
    const char* toReverse = "Programming";
    char* reversed = reverseString(toReverse);
    printf("Original: %s\\n", toReverse);
    printf("Reversed: %s\\n", reversed);
    printf("\\n");
    free(reversed);
    
    // Test Case 4: Case Conversion
    printf("Test Case 4 - Case Conversion:\\n");
    const char* mixedCase = "HeLLo WoRLd";
    char* upper = toUpperCaseSafe(mixedCase);
    char* lower = toLowerCaseSafe(mixedCase);
    printf("Original: %s\\n", mixedCase);
    printf("Uppercase: %s\\n", upper);
    printf("Lowercase: %s\\n", lower);
    printf("\\n");
    free(upper);
    free(lower);
    
    // Test Case 5: Substring and Replace
    printf("Test Case 5 - Substring and Replace:\\n");
    const char* text = "The quick brown fox jumps over the lazy dog";
    char* substring = getSubstringSafe(text, 4, 9);
    char* replaced = replaceSubstring(text, "fox", "cat");
    printf("Original: %s\\n", text);
    printf("Substring (4-9): %s\\n", substring);
    printf("Replaced 'fox' with 'cat': %s\\n", replaced);
    free(substring);
    free(replaced);
    
    return 0;
}
