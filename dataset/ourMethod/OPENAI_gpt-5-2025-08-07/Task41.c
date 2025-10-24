#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT 4096

// Reads a single line from stdin up to max_len characters (excluding the terminating null).
// Returns a heap-allocated string (caller must free). Sets *truncated_out to 1 if input
// exceeded max_len and was truncated; otherwise sets it to 0.
char* read_line_limited(size_t max_len, int* truncated_out) {
    if (truncated_out != NULL) {
        *truncated_out = 0;
    }

    // Allocate buffer for max_len characters + null terminator
    char* buf = (char*)malloc(max_len + 1);
    if (buf == NULL) {
        return NULL;
    }

    if (fgets(buf, (int)(max_len + 1), stdin) == NULL) {
        // EOF or error; return empty string
        buf[0] = '\0';
        return buf;
    }

    size_t got = strnlen(buf, max_len + 1);
    char* nl = strchr(buf, '\n');
    int truncated = 0;

    if (nl != NULL) {
        *nl = '\0';
    } else {
        // No newline found in buffer
        if (got == max_len) {
            truncated = 1;
        }
        // Discard the rest of the line to keep stdin clean
        int ch;
        while ((ch = fgetc(stdin)) != '\n' && ch != EOF) {
            truncated = 1;
        }
    }

    if (truncated_out != NULL) {
        *truncated_out = truncated;
    }

    return buf;
}

// Processes input by:
// - Removing non-printable characters (except whitespace is treated as a separator)
// - Trimming leading/trailing whitespace
// - Collapsing internal whitespace to a single space
// - Converting to uppercase
// Returns a newly allocated string; caller must free.
// The function processes at most MAX_INPUT characters to avoid excessive memory use.
char* process_string(const char* input) {
    if (input == NULL) {
        return NULL;
    }

    // Limit processing to MAX_INPUT characters
    size_t in_len = strnlen(input, MAX_INPUT);

    // Allocate output buffer with size <= input size + 1 (safe upper bound)
    char* out = (char*)calloc(in_len + 1, 1);
    if (out == NULL) {
        return NULL;
    }

    size_t o = 0;
    int in_space = 1; // treat as if starting after a space to skip leading spaces

    for (size_t i = 0; i < in_len; ++i) {
        unsigned char uc = (unsigned char)input[i];

        // Normalize CR/LF to space
        if (uc == '\r' || uc == '\n') {
            uc = ' ';
        }

        if (isspace(uc)) {
            in_space = 1;
            continue;
        }

        if (!isprint(uc)) {
            continue; // skip non-printable characters
        }

        if (in_space && o > 0) {
            out[o++] = ' ';
        }
        in_space = 0;

        out[o++] = (char)toupper(uc);
    }

    // out is already null-terminated due to calloc and controlled writes
    out[o] = '\0';
    return out;
}

int main(void) {
    // Self-tests (5 cases)
    const char* tests[5];
    tests[0] = "";
    tests[1] = "   hello   world   ";
    tests[2] = "a\tb\tc";
    tests[3] = "Hi\x01 there!\nNew line";
    // Build a long test case
    char* long_test = (char*)malloc(5001);
    if (long_test == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }
    memset(long_test, 'x', 5000);
    long_test[5000] = '\0';
    tests[4] = long_test;

    printf("Self-tests:\n");
    for (int i = 0; i < 5; ++i) {
        char* processed = process_string(tests[i]);
        if (processed == NULL) {
            printf("Test %d: [ERROR]\n", i + 1);
        } else {
            printf("Test %d: [%s]\n", i + 1, processed);
            free(processed);
        }
    }

    // User input
    printf("\nEnter a single line to process (max %d characters):\n", (int)MAX_INPUT);
    int truncated = 0;
    char* user = read_line_limited(MAX_INPUT, &truncated);
    if (user == NULL) {
        fprintf(stderr, "Failed to read input.\n");
        free(long_test);
        return 1;
    }
    if (truncated) {
        printf("Warning: input exceeded %d characters and was truncated.\n", (int)MAX_INPUT);
    }

    char* result = process_string(user);
    if (result == NULL) {
        fprintf(stderr, "Processing failed.\n");
        free(user);
        free(long_test);
        return 1;
    }

    printf("Processed: [%s]\n", result);

    free(result);
    free(user);
    free(long_test);
    return 0;
}