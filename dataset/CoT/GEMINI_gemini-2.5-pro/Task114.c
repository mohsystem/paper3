#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Creates a copy of a string.
 * The caller is responsible for freeing the returned memory.
 * @param src The source string.
 * @return A pointer to the newly allocated copy, or NULL on error.
 */
char* copyString(const char* src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = strlen(src) + 1;
    char* dst = (char*)malloc(len);
    if (dst == NULL) {
        return NULL; // Allocation failed
    }
    memcpy(dst, src, len);
    return dst;
}

/**
 * Concatenates two strings.
 * The caller is responsible for freeing the returned memory.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A pointer to the newly allocated concatenated string, or NULL on error.
 */
char* concatenateStrings(const char* s1, const char* s2) {
    if (s1 == NULL) s1 = "";
    if (s2 == NULL) s2 = "";
    
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char* result = (char*)malloc(len1 + len2 + 1);
    if (result == NULL) {
        return NULL; // Allocation failed
    }
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1); // Include null terminator from s2
    return result;
}

/**
 * Gets the length of a string.
 * @param s The input string.
 * @return The length of the string.
 */
size_t getStringLength(const char* s) {
    if (s == NULL) {
        return 0;
    }
    return strlen(s);
}

/**
 * Finds the first occurrence of a substring.
 * @param mainStr The string to search in.
 * @param subStr The substring to search for.
 * @return The starting index of the substring, or -1 if not found.
 */
int findSubstring(const char* mainStr, const char* subStr) {
    if (mainStr == NULL || subStr == NULL) {
        return -1;
    }
    char* found = strstr(mainStr, subStr);
    if (found == NULL) {
        return -1;
    }
    return (int)(found - mainStr);
}

/**
 * Replaces all occurrences of a substring with a new one.
 * The caller is responsible for freeing the returned memory.
 * @param mainStr The original string.
 * @param oldSub The substring to replace.
 * @param newSub The new substring.
 * @return A pointer to a new string with replacements, or NULL on error.
 */
char* replaceSubstring(const char* mainStr, const char* oldSub, const char* newSub) {
    if (!mainStr || !oldSub || !newSub) {
        return NULL;
    }

    size_t oldSubLen = strlen(oldSub);
    if (oldSubLen == 0) { // Avoid infinite loops on empty oldSub
        return copyString(mainStr);
    }
    size_t newSubLen = strlen(newSub);

    int count = 0;
    for (const char* p = mainStr; (p = strstr(p, oldSub)); p += oldSubLen) {
        count++;
    }

    size_t newLen = strlen(mainStr) - count * oldSubLen + count * newSubLen;
    char* result = (char*)malloc(newLen + 1);
    if (!result) {
        return NULL;
    }

    char* current_pos = result;
    const char* last_pos = mainStr;
    const char* next_occurrence;

    while ((next_occurrence = strstr(last_pos, oldSub))) {
        size_t non_match_len = next_occurrence - last_pos;
        memcpy(current_pos, last_pos, non_match_len);
        current_pos += non_match_len;
        memcpy(current_pos, newSub, newSubLen);
        current_pos += newSubLen;
        last_pos = next_occurrence + oldSubLen;
    }
    strcpy(current_pos, last_pos);
    return result;
}


int main() {
    // Test Case 1: Copying a string
    const char* original1 = "Hello World";
    char* copied1 = copyString(original1);
    printf("Test Case 1: Copying a String\n");
    printf("Original:  \"%s\"\n", original1);
    if(copied1) printf("Copied:    \"%s\"\n\n", copied1);
    free(copied1);

    // Test Case 2: Concatenating strings
    const char* s1 = "Hello, ";
    const char* s2 = "World!";
    char* concatenated = concatenateStrings(s1, s2);
    printf("Test Case 2: Concatenating Strings\n");
    printf("String 1: \"%s\"\n", s1);
    printf("String 2: \"%s\"\n", s2);
    if(concatenated) printf("Concatenated: \"%s\"\n\n", concatenated);
    free(concatenated);

    // Test Case 3: Getting string length
    const char* s3 = "Programming";
    size_t length = getStringLength(s3);
    printf("Test Case 3: Getting String Length\n");
    printf("String: \"%s\"\n", s3);
    printf("Length: %zu\n\n", length);

    // Test Case 4: Finding a substring
    const char* mainStr4 = "This is a test";
    const char* subStr4 = "is";
    int index = findSubstring(mainStr4, subStr4);
    printf("Test Case 4: Finding a Substring\n");
    printf("Main String: \"%s\"\n", mainStr4);
    printf("Substring:   \"%s\"\n", subStr4);
    printf("Found at index: %d\n\n", index);

    // Test Case 5: Replacing a substring
    const char* mainStr5 = "The house is blue and the car is blue.";
    const char* oldSub5 = "blue";
    const char* newSub5 = "red";
    char* replaced = replaceSubstring(mainStr5, oldSub5, newSub5);
    printf("Test Case 5: Replacing a Substring\n");
    printf("Original: \"%s\"\n", mainStr5);
    if (replaced) printf("Replaced: \"%s\"\n\n", replaced);
    free(replaced);
    
    return 0;
}