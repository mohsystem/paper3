
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// String copy operation
char* copyString(const char* source) {
    if (source == NULL) {
        return NULL;
    }
    size_t len = strlen(source);
    char* copy = (char*)malloc((len + 1) * sizeof(char));
    if (copy != NULL) {
        strcpy(copy, source);
    }
    return copy;
}

// String concatenation operation
char* concatenateStrings(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL) {
        return NULL;
    }
    size_t len1 = (str1 != NULL) ? strlen(str1) : 0;
    size_t len2 = (str2 != NULL) ? strlen(str2) : 0;
    
    char* result = (char*)malloc((len1 + len2 + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    result[0] = '\\0';
    if (str1 != NULL) {
        strcpy(result, str1);
    }
    if (str2 != NULL) {
        strcat(result, str2);
    }
    return result;
}

// String reverse operation
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

// String uppercase operation
char* toUpperCaseString(const char* input) {
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

// String lowercase operation
char* toLowerCaseString(const char* input) {
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

// String substring operation
char* getSubstring(const char* input, int start, int end) {
    if (input == NULL || start < 0 || start > end) {
        return NULL;
    }
    int len = strlen(input);
    if (end > len) {
        end = len;
    }
    
    int subLen = end - start;
    char* substring = (char*)malloc((subLen + 1) * sizeof(char));
    if (substring == NULL) {
        return NULL;
    }
    
    strncpy(substring, input + start, subLen);
    substring[subLen] = '\\0';
    return substring;
}

// String replace operation
char* replaceString(const char* input, const char* target, const char* replacement) {
    if (input == NULL || target == NULL || replacement == NULL) {
        return NULL;
    }
    
    size_t inputLen = strlen(input);
    size_t targetLen = strlen(target);
    size_t replacementLen = strlen(replacement);
    
    // Count occurrences
    int count = 0;
    const char* temp = input;
    while ((temp = strstr(temp, target)) != NULL) {
        count++;
        temp += targetLen;
    }
    
    // Allocate result buffer
    size_t resultLen = inputLen + count * (replacementLen - targetLen);
    char* result = (char*)malloc((resultLen + 1) * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    // Perform replacement
    char* resultPtr = result;
    const char* inputPtr = input;
    while (*inputPtr) {
        if (strstr(inputPtr, target) == inputPtr) {
            strcpy(resultPtr, replacement);
            resultPtr += replacementLen;
            inputPtr += targetLen;
        } else {
            *resultPtr++ = *inputPtr++;
        }
    }
    *resultPtr = '\\0';
    
    return result;
}

// String length operation
int getStringLength(const char* input) {
    if (input == NULL) {
        return 0;
    }
    return strlen(input);
}

int main() {
    // Test Case 1: Copy string
    printf("Test Case 1 - Copy String:\\n");
    const char* original = "Hello World";
    char* copied = copyString(original);
    printf("Original: %s\\n", original);
    printf("Copied: %s\\n", copied);
    printf("\\n");
    free(copied);
    
    // Test Case 2: Concatenate strings
    printf("Test Case 2 - Concatenate Strings:\\n");
    const char* str1 = "Hello";
    const char* str2 = " World";
    char* concatenated = concatenateStrings(str1, str2);
    printf("String 1: %s\\n", str1);
    printf("String 2: %s\\n", str2);
    printf("Concatenated: %s\\n", concatenated);
    printf("\\n");
    free(concatenated);
    
    // Test Case 3: Reverse string
    printf("Test Case 3 - Reverse String:\\n");
    const char* text = "Programming";
    char* reversed = reverseString(text);
    printf("Original: %s\\n", text);
    printf("Reversed: %s\\n", reversed);
    printf("\\n");
    free(reversed);
    
    // Test Case 4: Case conversion and substring
    printf("Test Case 4 - Case Conversion and Substring:\\n");
    const char* message = "Secure Coding Practice";
    char* upper = toUpperCaseString(message);
    char* lower = toLowerCaseString(message);
    char* substring = getSubstring(message, 0, 6);
    printf("Original: %s\\n", message);
    printf("Uppercase: %s\\n", upper);
    printf("Lowercase: %s\\n", lower);
    printf("Substring (0-6): %s\\n", substring);
    printf("\\n");
    free(upper);
    free(lower);
    free(substring);
    
    // Test Case 5: Replace and length
    printf("Test Case 5 - Replace and Length:\\n");
    const char* sentence = "Java is great. Java is powerful.";
    char* replaced = replaceString(sentence, "Java", "Python");
    printf("Original: %s\\n", sentence);
    printf("Replaced: %s\\n", replaced);
    printf("Original Length: %d\\n", getStringLength(sentence));
    printf("Replaced Length: %d\\n", getStringLength(replaced));
    free(replaced);
    
    return 0;
}
