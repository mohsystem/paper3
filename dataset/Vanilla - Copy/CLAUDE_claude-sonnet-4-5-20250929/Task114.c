
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Copy string
char* copyString(const char* source) {
    if (source == NULL) return NULL;
    char* copy = (char*)malloc(strlen(source) + 1);
    strcpy(copy, source);
    return copy;
}

// Concatenate two strings
char* concatenateStrings(const char* str1, const char* str2) {
    if (str1 == NULL) str1 = "";
    if (str2 == NULL) str2 = "";
    char* result = (char*)malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

// Reverse a string
char* reverseString(const char* str) {
    if (str == NULL) return NULL;
    int len = strlen(str);
    char* reversed = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        reversed[i] = str[len - 1 - i];
    }
    reversed[len] = '\\0';
    return reversed;
}

// Convert to uppercase
char* toUpperCase(const char* str) {
    if (str == NULL) return NULL;
    int len = strlen(str);
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = toupper(str[i]);
    }
    result[len] = '\\0';
    return result;
}

// Convert to lowercase
char* toLowerCase(const char* str) {
    if (str == NULL) return NULL;
    int len = strlen(str);
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = tolower(str[i]);
    }
    result[len] = '\\0';
    return result;
}

// Get substring
char* getSubstring(const char* str, int start, int end) {
    if (str == NULL || start < 0 || end > strlen(str) || start > end) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    int len = end - start;
    char* result = (char*)malloc(len + 1);
    strncpy(result, str + start, len);
    result[len] = '\\0';
    return result;
}

// Count character occurrences
int countCharacter(const char* str, char ch) {
    if (str == NULL) return 0;
    int count = 0;
    for (int i = 0; str[i] != '\\0'; i++) {
        if (str[i] == ch) {
            count++;
        }
    }
    return count;
}

// Replace character
char* replaceCharacter(const char* str, char oldChar, char newChar) {
    if (str == NULL) return NULL;
    int len = strlen(str);
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = (str[i] == oldChar) ? newChar : str[i];
    }
    result[len] = '\\0';
    return result;
}

// Find string length
int getLength(const char* str) {
    if (str == NULL) return 0;
    return strlen(str);
}

// Check if string is palindrome
int isPalindrome(const char* str) {
    if (str == NULL) return 0;
    char* reversed = reverseString(str);
    int result = strcmp(str, reversed) == 0;
    free(reversed);
    return result;
}

int main() {
    printf("=== Test Case 1: Copy and Concatenate ===\\n");
    const char* original = "Hello";
    char* copied = copyString(original);
    printf("Original: %s\\n", original);
    printf("Copied: %s\\n", copied);
    char* concatenated = concatenateStrings("Hello", " World");
    printf("Concatenated: %s\\n", concatenated);
    free(copied);
    free(concatenated);
    
    printf("\\n=== Test Case 2: Reverse String ===\\n");
    const char* str2 = "Programming";
    printf("Original: %s\\n", str2);
    char* reversed = reverseString(str2);
    printf("Reversed: %s\\n", reversed);
    free(reversed);
    
    printf("\\n=== Test Case 3: Case Conversion ===\\n");
    const char* str3 = "Java Python CPP";
    printf("Original: %s\\n", str3);
    char* upper = toUpperCase(str3);
    char* lower = toLowerCase(str3);
    printf("Uppercase: %s\\n", upper);
    printf("Lowercase: %s\\n", lower);
    free(upper);
    free(lower);
    
    printf("\\n=== Test Case 4: Substring and Character Operations ===\\n");
    const char* str4 = "Hello World";
    printf("Original: %s\\n", str4);
    char* substr = getSubstring(str4, 0, 5);
    printf("Substring (0, 5): %s\\n", substr);
    printf("Count 'l': %d\\n", countCharacter(str4, 'l'));
    char* replaced = replaceCharacter(str4, 'o', '0');
    printf("Replace 'o' with '0': %s\\n", replaced);
    printf("Length: %d\\n", getLength(str4));
    free(substr);
    free(replaced);
    
    printf("\\n=== Test Case 5: Palindrome Check ===\\n");
    const char* str5a = "radar";
    const char* str5b = "hello";
    printf("%s is palindrome: %s\\n", str5a, isPalindrome(str5a) ? "true" : "false");
    printf("%s is palindrome: %s\\n", str5b, isPalindrome(str5b) ? "true" : "false");
    
    return 0;
}
