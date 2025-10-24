#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Creates a dynamically allocated copy of a string.
 * The caller is responsible for freeing the returned memory.
 * @param src The source string.
 * @return A pointer to the newly allocated copied string.
 */
char* copyString(const char* src) {
    if (src == NULL) return NULL;
    char* dest = malloc(strlen(src) + 1);
    if (dest == NULL) return NULL;
    strcpy(dest, src);
    return dest;
}

/**
 * Concatenates two strings into a new dynamically allocated string.
 * The caller is responsible for freeing the returned memory.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A pointer to the newly allocated concatenated string.
 */
char* concatenateStrings(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) return NULL;
    char* result = malloc(strlen(s1) + strlen(s2) + 1);
    if (result == NULL) return NULL;
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

/**
 * Gets the length of a string.
 * @param s The input string.
 * @return The length of the string.
 */
size_t getStringLength(const char* s) {
    return strlen(s);
}

/**
 * Finds the index of a substring within a main string.
 * @param mainStr The string to search in.
 * @param subStr The substring to search for.
 * @return The starting index of the substring, or -1 if not found.
 */
int findSubstring(const char* mainStr, const char* subStr) {
    char* found = strstr(mainStr, subStr);
    if (found == NULL) {
        return -1;
    }
    return (int)(found - mainStr);
}

/**
 * Converts a string to uppercase, returning a new dynamically allocated string.
 * The caller is responsible for freeing the returned memory.
 * @param s The input string.
 * @return A pointer to the new uppercase string.
 */
char* toUpperCase(const char* s) {
    if (s == NULL) return NULL;
    char* result = copyString(s);
    if (result == NULL) return NULL;
    for (int i = 0; result[i]; i++) {
        result[i] = toupper((unsigned char)result[i]);
    }
    return result;
}

/**
 * Converts a string to lowercase, returning a new dynamically allocated string.
 * The caller is responsible for freeing the returned memory.
 * @param s The input string.
 * @return A pointer to the new lowercase string.
 */
char* toLowerCase(const char* s) {
    if (s == NULL) return NULL;
    char* result = copyString(s);
    if (result == NULL) return NULL;
    for (int i = 0; result[i]; i++) {
        result[i] = tolower((unsigned char)result[i]);
    }
    return result;
}


int main() {
    const char* testCases[][2] = {
        {"Hello", "World"},
        {"C Language", "Strings"},
        {"Test", "Case"},
        {"one two three", "two"},
        {"UPPER", "lower"}
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        const char* s1 = testCases[i][0];
        const char* s2 = testCases[i][1];

        printf("--- Test Case %d ---\n", i + 1);
        printf("Original strings: \"%s\", \"%s\"\n", s1, s2);

        // Copy
        char* copied = copyString(s1);
        printf("Copied s1: %s\n", copied);
        free(copied);

        // Concatenate
        char* concatenated = concatenateStrings(s1, s2);
        printf("Concatenated: %s\n", concatenated);
        free(concatenated);

        // Length
        printf("Length of s1: %zu\n", getStringLength(s1));

        // Find Substring
        int index;
        if (strcmp(s1, "one two three") == 0 && strcmp(s2, "two") == 0) { // special case
            index = findSubstring(s1, s2);
            printf("Index of \"%s\" in \"%s\": %d\n", s2, s1, index);
        } else {
            index = findSubstring(s1, "o");
            printf("Index of 'o' in \"%s\": %d\n", s1, index);
        }

        // To Uppercase
        char* upper = toUpperCase(s1);
        printf("s1 to uppercase: %s\n", upper);
        free(upper);

        // To Lowercase
        char* lower = toLowerCase(s2);
        printf("s2 to lowercase: %s\n", lower);
        free(lower);
        
        printf("\n");
    }

    return 0;
}