#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// Checks to see if a string has the same amount of 'x's and 'o's.
// The function is case insensitive. The string can contain any char.
bool xo(const char* str) {
    // A NULL pointer can be treated as a string with 0 'x's and 0 'o's.
    if (str == NULL) {
        return true;
    }

    int x_count = 0;
    int o_count = 0;

    for (int i = 0; str[i] != '\0'; ++i) {
        // Use tolower for case-insensitivity.
        // Cast to unsigned char is the standard safe way to use ctype functions.
        char lower_c = tolower((unsigned char)str[i]);
        if (lower_c == 'x') {
            x_count++;
        } else if (lower_c == 'o') {
            o_count++;
        }
    }

    return x_count == o_count;
}

int main() {
    const char* test_cases[] = {
        "ooxx",
        "xooxx",
        "ooxXm",
        "zpzpzpp",
        "zzoo"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_test_cases; ++i) {
        const char* test_str = test_cases[i];
        bool result = xo(test_str);
        printf("XO(\"%s\") => %s\n", test_str, result ? "true" : "false");
    }

    return 0;
}