#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Checks if a string ends with a given suffix.
 *
 * @param str The main string to check. Must be a valid null-terminated string.
 * @param ending The suffix to look for. Must be a valid null-terminated string.
 * @return true if str ends with ending, false otherwise. Returns false if either input is NULL.
 */
bool solution(const char *str, const char *ending) {
    // Validate inputs: treat NULL pointers as untrusted input.
    if (str == NULL || ending == NULL) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);

    // If the ending is longer than the string, it can't be a suffix.
    // This also prevents underflow with unsigned size_t arithmetic.
    if (ending_len > str_len) {
        return false;
    }

    // Calculate the starting position of the potential suffix in str.
    const char *suffix_start = str + (str_len - ending_len);

    // Compare the suffix of str with the ending string.
    // strcmp returns 0 if the strings are identical.
    return strcmp(suffix_start, ending) == 0;
}

int main() {
    // Test cases
    const char* test_strs[] = {"abc", "abc", "samurai", "testing", "fails"};
    const char* test_endings[] = {"bc", "d", "ai", "", "ails"};
    const int num_tests = 5;

    printf("Running C test cases...\n");
    for (int i = 0; i < num_tests; ++i) {
        bool result = solution(test_strs[i], test_endings[i]);
        printf("solution(\"%s\", \"%s\") -> %s\n",
               test_strs[i], test_endings[i], result ? "true" : "false");
    }

    return 0;
}