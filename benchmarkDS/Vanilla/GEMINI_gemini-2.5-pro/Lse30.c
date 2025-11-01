#include <stdio.h>
#include <string.h>
#include <regex.h>

/*
 * Note: This code uses the POSIX regex library. When compiling with gcc/clang,
 * it usually links automatically with the standard C library.
 */

/**
 * @brief Determines the redirect URL based on a regex match and writes it to a result buffer.
 * The regex matches a string that starts with one or more alphanumeric characters or hyphens,
 * followed by a period.
 * 
 * @param target The target query parameter string.
 * @param result A character buffer to store the output string.
 * @param result_size The size of the result buffer.
 */
void getRedirectUrl(const char* target, char* result, size_t result_size) {
    regex_t regex;
    int status;
    const char* pattern = "^[a-zA-Z0-9-]+\\..*";

    // Compile the regular expression
    status = regcomp(&regex, pattern, REG_EXTENDED);
    if (status) {
        fprintf(stderr, "Could not compile regex\n");
        snprintf(result, result_size, "Regex compilation error.");
        return;
    }

    // Execute the regular expression
    status = regexec(&regex, target, 0, NULL, 0);

    if (status == 0) {
        // Match found
        snprintf(result, result_size, "Redirecting to: %s", target);
    } else if (status == REG_NOMATCH) {
        // No match
        snprintf(result, result_size, "Redirecting to: /");
    } else {
        char msgbuf[100];
        regerror(status, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        snprintf(result, result_size, "Regex match error.");
    }

    // Free the memory allocated for the compiled regex
    regfree(&regex);
}

int main() {
    const char* testCases[] = {
        "example.com",
        "sub-domain.example.org/path?query=1",
        "123-valid.net",
        ".invalid-start.com",
        "/local/path/only"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    char result[256];

    printf("C Test Cases:\n");
    for (int i = 0; i < numTestCases; i++) {
        getRedirectUrl(testCases[i], result, sizeof(result));
        printf("Input: \"%s\" -> Output: %s\n", testCases[i], result);
    }

    return 0;
}