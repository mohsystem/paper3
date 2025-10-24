#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Checks if the first string ends with the second string.
 *
 * @param str The main null-terminated string.
 * @param ending The null-terminated string to check for at the end of str.
 * @return true if str ends with ending, false otherwise.
 */
bool solution(const char *str, const char *ending) {
    if (str == NULL || ending == NULL) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);

    if (ending_len > str_len) {
        return false;
    }

    // Compare the substring of str with ending
    // The substring starts at the position (str_len - ending_len)
    return strcmp(str + (str_len - ending_len), ending) == 0;
}

int main() {
    // Helper function to print boolean as string
    const char* bool_to_string(bool b) {
        return b ? "true" : "false";
    }

    // Test cases
    printf("Test Case 1: solution(\"abc\", \"bc\")\n");
    printf("Expected: true\n");
    printf("Actual: %s\n\n", bool_to_string(solution("abc", "bc")));

    printf("Test Case 2: solution(\"abc\", \"d\")\n");
    printf("Expected: false\n");
    printf("Actual: %s\n\n", bool_to_string(solution("abc", "d")));

    printf("Test Case 3: solution(\"samurai\", \"ai\")\n");
    printf("Expected: true\n");
    printf("Actual: %s\n\n", bool_to_string(solution("samurai", "ai")));
    
    printf("Test Case 4: solution(\"abc\", \"\")\n");
    printf("Expected: true\n");
    printf("Actual: %s\n\n", bool_to_string(solution("abc", "")));

    printf("Test Case 5: solution(\"abc\", \"abcde\")\n");
    printf("Expected: false\n");
    printf("Actual: %s\n\n", bool_to_string(solution("abc", "abcde")));

    return 0;
}