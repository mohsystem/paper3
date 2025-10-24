#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Securely creates a copy of a string by allocating new memory.
 * Checks for null input and malloc failure.
 * @param src The source string to copy.
 * @return A pointer to the newly allocated string, or NULL on failure.
 *         The caller is responsible for freeing this memory.
 */
char* secureStringCopy(const char* src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = strlen(src);
    // Allocate memory for the new string (+1 for the null terminator)
    char* dest = (char*)malloc(len + 1);
    if (dest == NULL) {
        perror("Failed to allocate memory for string copy");
        return NULL;
    }
    // Use memcpy for safe, non-overlapping memory copy
    memcpy(dest, src, len + 1);
    return dest;
}

/**
 * @brief Securely concatenates two strings into a new buffer.
 * Checks for null inputs and malloc failure.
 * @param s1 The first string.
 * @param s2 The second string.
 * @return A pointer to the newly allocated concatenated string, or NULL on failure.
 *         The caller is responsible for freeing this memory.
 */
char* secureStringConcat(const char* s1, const char* s2) {
    if (s1 == NULL && s2 == NULL) {
        return NULL;
    }
    size_t len1 = (s1 == NULL) ? 0 : strlen(s1);
    size_t len2 = (s2 == NULL) ? 0 : strlen(s2);

    char* result = (char*)malloc(len1 + len2 + 1);
    if (result == NULL) {
        perror("Failed to allocate memory for string concatenation");
        return NULL;
    }

    if (s1 != NULL) {
        memcpy(result, s1, len1);
    }
    if (s2 != NULL) {
        memcpy(result + len1, s2, len2);
    }
    
    result[len1 + len2] = '\0';
    return result;
}

/**
 * @brief Creates a reversed copy of a string.
 * Checks for null input and malloc failure.
 * @param src The source string to reverse.
 * @return A pointer to the newly allocated reversed string, or NULL on failure.
 *         The caller is responsible for freeing this memory.
 */
char* secureStringReverse(const char* src) {
    if (src == NULL) {
        return NULL;
    }
    size_t len = strlen(src);
    char* reversed = (char*)malloc(len + 1);
    if (reversed == NULL) {
        perror("Failed to allocate memory for string reverse");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        reversed[i] = src[len - 1 - i];
    }
    reversed[len] = '\0';
    return reversed;
}


int main() {
    // Test Case 1: Basic operations
    printf("--- Test Case 1: Basic Operations ---\n");
    const char* str1 = "hello";
    const char* str2 = "world";

    char* copiedStr = secureStringCopy(str1);
    if (copiedStr) {
        printf("Original: %s, Copied: %s\n", str1, copiedStr);
        free(copiedStr);
    }

    char* concatenatedStr = secureStringConcat(str1, str2);
    if (concatenatedStr) {
        printf("Concatenated: \"%s\" + \"%s\" = \"%s\"\n", str1, str2, concatenatedStr);
        free(concatenatedStr);
    }

    char* reversedStr = secureStringReverse("C Language");
    if (reversedStr) {
        printf("Original: C Language, Reversed: %s\n", reversedStr);
        free(reversedStr);
    }
    printf("\n");

    // Test Case 2: Empty strings
    printf("--- Test Case 2: Empty Strings ---\n");
    const char* emptyStr = "";
    const char* textStr = "test";
    
    copiedStr = secureStringCopy(emptyStr);
    if (copiedStr) {
        printf("Original: \"\", Copied: \"%s\"\n", copiedStr);
        free(copiedStr);
    }

    concatenatedStr = secureStringConcat(emptyStr, textStr);
    if (concatenatedStr) {
        printf("Concatenated: \"\" + \"%s\" = \"%s\"\n", textStr, concatenatedStr);
        free(concatenatedStr);
    }

    reversedStr = secureStringReverse(emptyStr);
    if (reversedStr) {
        printf("Original: \"\", Reversed: \"%s\"\n", reversedStr);
        free(reversedStr);
    }
    printf("\n");

    // Test Case 3: Null inputs
    printf("--- Test Case 3: Null Inputs ---\n");
    copiedStr = secureStringCopy(NULL);
    printf("Copying NULL: %s\n", (copiedStr == NULL ? "(null)" : "ERROR"));
    free(copiedStr); // free(NULL) is safe

    concatenatedStr = secureStringConcat(NULL, "safe");
    if (concatenatedStr) {
        printf("Concatenating NULL and \"safe\": %s\n", concatenatedStr);
        free(concatenatedStr);
    }
    
    reversedStr = secureStringReverse(NULL);
    printf("Reversing NULL: %s\n", (reversedStr == NULL ? "(null)" : "ERROR"));
    free(reversedStr); // free(NULL) is safe
    printf("\n");

    // Test Case 4: Strings with spaces and special characters
    printf("--- Test Case 4: Special Characters ---\n");
    const char* specialStr = " A B C!@#123 ";
    copiedStr = secureStringCopy(specialStr);
    if (copiedStr) {
        printf("Original: \"%s\", Copied: \"%s\"\n", specialStr, copiedStr);
        free(copiedStr);
    }

    concatenatedStr = secureStringConcat(specialStr, "end");
    if (concatenatedStr) {
        printf("Concatenated: \"%s\" + \"end\" = \"%s\"\n", specialStr, concatenatedStr);
        free(concatenatedStr);
    }

    reversedStr = secureStringReverse(specialStr);
    if (reversedStr) {
        printf("Original: \"%s\", Reversed: \"%s\"\n", specialStr, reversedStr);
        free(reversedStr);
    }
    printf("\n");

    // Test Case 5: Single character string
    printf("--- Test Case 5: Single Character String ---\n");
    const char* singleCharStr = "a";
    copiedStr = secureStringCopy(singleCharStr);
    if (copiedStr) {
        printf("Original: %s, Copied: %s\n", singleCharStr, copiedStr);
        free(copiedStr);
    }

    concatenatedStr = secureStringConcat(singleCharStr, "b");
    if (concatenatedStr) {
        printf("Concatenated: \"%s\" + \"b\" = \"%s\"\n", singleCharStr, concatenatedStr);
        free(concatenatedStr);
    }

    reversedStr = secureStringReverse(singleCharStr);
    if (reversedStr) {
        printf("Original: %s, Reversed: %s\n", singleCharStr, reversedStr);
        free(reversedStr);
    }
    
    return 0;
}