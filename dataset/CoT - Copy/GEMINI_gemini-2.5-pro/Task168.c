#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tweaks letters in a string based on an array of integers.
 *        The caller is responsible for freeing the returned string.
 * 
 * @param str The input null-terminated string (all lowercase letters).
 * @param arr An array of integers (0, 1, or -1). Must have at least strlen(str) elements.
 * @return char* A new heap-allocated string with the tweaked letters, or NULL on failure.
 */
char* tweakLetters(const char* str, const int* arr) {
    if (str == NULL || arr == NULL) {
        return NULL;
    }
    
    size_t len = strlen(str);
    char* result = (char*)malloc(len + 1);
    
    if (result == NULL) {
        perror("Failed to allocate memory for result string");
        return NULL;
    }
    
    for (size_t i = 0; i < len; ++i) {
        result[i] = str[i] + arr[i];
    }
    result[len] = '\0';
    
    return result;
}

int main() {
    // Test Case 1
    const char* str1 = "apple";
    int arr1[] = {0, 1, -1, 0, -1};
    char* res1 = tweakLetters(str1, arr1);
    if (res1) {
        printf("tweakLetters(\"%s\", {0, 1, -1, 0, -1}) -> \"%s\"\n", str1, res1);
        free(res1);
    }

    // Test Case 2
    const char* str2 = "many";
    int arr2[] = {0, 0, 0, -1};
    char* res2 = tweakLetters(str2, arr2);
    if (res2) {
        printf("tweakLetters(\"%s\", {0, 0, 0, -1}) -> \"%s\"\n", str2, res2);
        free(res2);
    }
    
    // Test Case 3
    const char* str3 = "rhino";
    int arr3[] = {1, 1, 1, 1, 1};
    char* res3 = tweakLetters(str3, arr3);
    if (res3) {
        printf("tweakLetters(\"%s\", {1, 1, 1, 1, 1}) -> \"%s\"\n", str3, res3);
        free(res3);
    }

    // Test Case 4
    const char* str4 = "abc";
    int arr4[] = {-1, 0, 1};
    char* res4 = tweakLetters(str4, arr4);
    if (res4) {
        printf("tweakLetters(\"%s\", {-1, 0, 1}) -> \"%s\"\n", str4, res4);
        free(res4);
    }

    // Test Case 5
    const char* str5 = "zulu";
    int arr5[] = {1, -1, 1, -1};
    char* res5 = tweakLetters(str5, arr5);
    if (res5) {
        printf("tweakLetters(\"%s\", {1, -1, 1, -1}) -> \"%s\"\n", str5, res5);
        free(res5);
    }

    return 0;
}