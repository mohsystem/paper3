#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * Checks if the first string ends with the second string.
 * This code is secure because it performs multiple checks to prevent common
 * vulnerabilities:
 * 1. It checks for NULL pointers for both inputs to prevent segmentation faults.
 * 2. It calculates the lengths and ensures the main string is at least as long
 *    as the ending string, preventing buffer under-reads.
 * 3. It uses `strcmp`, a standard library function, for safe string comparison.
 *
 * @param str The main null-terminated string.
 * @param ending The null-terminated string to check for at the end of str.
 * @return true if str ends with ending, false otherwise.
 */
bool solution(const char *str, const char *ending) {
    // 1. Securely handle NULL inputs
    if (str == NULL || ending == NULL) {
        return false;
    }

    size_t str_len = strlen(str);
    size_t ending_len = strlen(ending);

    // 2. Prevent reading out of bounds if ending is longer than str
    if (ending_len > str_len) {
        return false;
    }

    // 3. Find the potential start of the suffix in str and compare safely
    const char *start_of_suffix = str + (str_len - ending_len);
    return strcmp(start_of_suffix, ending) == 0;
}

int main() {
    // Test cases
    printf("('abc', 'bc') -> %s\n", solution("abc", "bc") ? "true" : "false"); // true
    printf("('abc', 'd') -> %s\n", solution("abc", "d") ? "true" : "false"); // false
    printf("('sushi', 'shi') -> %s\n", solution("sushi", "shi") ? "true" : "false"); // true
    printf("('samurai', 'ra') -> %s\n", solution("samurai", "ra") ? "true" : "false"); // false
    printf("('abc', '') -> %s\n", solution("abc", "") ? "true" : "false"); // true
    
    return 0;
}