#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Tweaks letters by one forward (+1) or backwards (-1) according to an array.
 * 
 * @param str The input string of lowercase letters.
 * @param tweaks An array of integers (0, 1, or -1).
 * @param len The length of the string and the tweaks array.
 * @return A new dynamically allocated string with the tweaked letters. The caller must free this string.
 */
char* tweakLetters(const char* str, const int* tweaks, size_t len) {
    char* result = (char*)malloc(len + 1);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        int tweak = tweaks[i];
        char newChar = c + tweak;

        if (newChar > 'z') {
            newChar = 'a';
        } else if (newChar < 'a') {
            newChar = 'z';
        }
        result[i] = newChar;
    }
    result[len] = '\0';
    return result;
}

void run_test(const char* str, const int* tweaks, size_t len) {
    char* result = tweakLetters(str, tweaks, len);
    if (result) {
        printf("%s\n", result);
        free(result);
    }
}

int main() {
    // Test Case 1
    int tweaks1[] = {0, 1, -1, 0, -1};
    run_test("apple", tweaks1, sizeof(tweaks1)/sizeof(int));

    // Test Case 2
    int tweaks2[] = {0, 0, 0, -1};
    run_test("many", tweaks2, sizeof(tweaks2)/sizeof(int));

    // Test Case 3
    int tweaks3[] = {1, 1, 1, 1, 1};
    run_test("rhino", tweaks3, sizeof(tweaks3)/sizeof(int));

    // Test Case 4
    int tweaks4[] = {1, -1, 1, -1, 1};
    run_test("zebra", tweaks4, sizeof(tweaks4)/sizeof(int));

    // Test Case 5
    int tweaks5[] = {-1, 0, 1};
    run_test("abc", tweaks5, sizeof(tweaks5)/sizeof(int));

    return 0;
}