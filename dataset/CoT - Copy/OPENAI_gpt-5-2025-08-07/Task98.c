/*
Chain-of-Through process:
1) Problem understanding: Read console input and attempt to convert to 32-bit int; also provide a function taking a parameter.
2) Security requirements: Limit input length, trim and validate characters, handle conversion errors and range safely.
3) Secure coding generation: Use strtol with errno, ensure full string consumption and range checks.
4) Code review: Checked for buffer sizes, safe string handling, and no dynamic memory leaks.
5) Secure code output: Final code includes safe parsing, console interaction, and 5 test cases.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_INPUT_LEN 1000
#define READ_BUFFER 2048

typedef struct {
    bool success;
    int value;
    const char* error;
} ParseResult;

static void trim_copy(const char* in, char* out, size_t out_size) {
    // out_size must be >= 1
    size_t len = strlen(in);
    size_t start = 0;
    while (start < len && isspace((unsigned char)in[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)in[end - 1])) end--;
    size_t to_copy = (end > start) ? (end - start) : 0;
    if (to_copy >= out_size) {
        to_copy = out_size - 1;
    }
    if (to_copy > 0) {
        memcpy(out, in + start, to_copy);
    }
    out[to_copy] = '\0';
}

// Function that accepts input as parameter and returns output
ParseResult try_parse_int(const char* input) {
    if (input == NULL) {
        ParseResult r = {false, 0, "Input is NULL"};
        return r;
    }

    char s[MAX_INPUT_LEN + 1]; // enforce maximum length
    // First trim into a temporary large buffer to check length
    char temp[READ_BUFFER];
    trim_copy(input, temp, sizeof(temp));

    size_t len = strlen(temp);
    if (len == 0) {
        ParseResult r = {false, 0, "Empty input"};
        return r;
    }
    if (len > MAX_INPUT_LEN) {
        ParseResult r = {false, 0, "Input too long"};
        return r;
    }
    // Safe copy as we've validated size
    memcpy(s, temp, len + 1);

    // Validate characters: optional sign followed by digits
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (i == 0 && (c == '+' || c == '-')) continue;
        if (!isdigit((unsigned char)c)) {
            ParseResult r = {false, 0, "Invalid characters"};
            return r;
        }
    }

    errno = 0;
    char* endptr = NULL;
    long val = strtol(s, &endptr, 10);
    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        ParseResult r = {false, 0, "Number out of int range"};
        return r;
    }
    if (endptr == NULL || *endptr != '\0') {
        ParseResult r = {false, 0, "Invalid characters"};
        return r;
    }

    ParseResult r = {true, (int)val, NULL};
    return r;
}

int main(void) {
    printf("Enter a value to convert to integer:\n");
    char buf[READ_BUFFER];
    if (fgets(buf, sizeof(buf), stdin) != NULL) {
        // Remove trailing newline if present
        size_t blen = strlen(buf);
        if (blen > 0 && buf[blen - 1] == '\n') {
            buf[blen - 1] = '\0';
        }
        ParseResult res = try_parse_int(buf);
        if (res.success) {
            printf("Success: %d\n", res.value);
        } else {
            printf("Error: %s\n", res.error);
        }
    } else {
        printf("No input received.\n");
    }

    // 5 test cases
    const char* tests[5] = {"123", "  -42  ", "2147483647", "2147483648", "abc"};
    printf("---- Test Cases ----\n");
    for (int i = 0; i < 5; ++i) {
        ParseResult r = try_parse_int(tests[i]);
        if (r.success) {
            printf("Input: \"%s\" -> Success: %d\n", tests[i], r.value);
        } else {
            printf("Input: \"%s\" -> Error: %s\n", tests[i], r.error);
        }
    }

    return 0;
}