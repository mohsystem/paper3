#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*
    Processes the input C-string by:
    - Removing non-whitespace control characters (ASCII < 32 or 127)
    - Collapsing consecutive whitespace (space or tab) into a single space
    - Trimming leading and trailing spaces
    Returns a newly allocated string (caller must free). Safe against buffer overflow.
*/
char* processInput(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    // First pass: compute required length
    size_t out_len = 0;
    int started = 0;
    int prev_space = 0;

    for (const unsigned char* p = (const unsigned char*)input; *p != '\0'; ++p) {
        unsigned char uc = *p;
        int is_whitespace = (uc == ' ' || uc == '\t');
        int is_control = (uc < 32 || uc == 127);

        if (is_control && !is_whitespace) {
            continue; // skip non-whitespace control chars
        }

        if (is_whitespace) {
            if (started && !prev_space) {
                // add a space
                if (out_len != SIZE_MAX) out_len++;
                prev_space = 1;
            }
        } else {
            if (out_len != SIZE_MAX) out_len++;
            started = 1;
            prev_space = 0;
        }
    }

    // Trim trailing space count if last was space
    if (out_len > 0 && prev_space) {
        out_len--;
    }

    // Allocate output buffer (+1 for NUL), check for overflow
    if (out_len >= SIZE_MAX) {
        return NULL; // size overflow
    }
    size_t alloc_size = out_len + 1;
    if (alloc_size < out_len) {
        return NULL; // overflow check
    }

    char* out = (char*)malloc(alloc_size);
    if (!out) return NULL;

    // Second pass: build output
    size_t idx = 0;
    started = 0;
    prev_space = 0;
    for (const unsigned char* p = (const unsigned char*)input; *p != '\0'; ++p) {
        unsigned char uc = *p;
        int is_whitespace = (uc == ' ' || uc == '\t');
        int is_control = (uc < 32 || uc == 127);

        if (is_control && !is_whitespace) {
            continue;
        }

        if (is_whitespace) {
            if (started && !prev_space) {
                if (idx < out_len) out[idx++] = ' ';
                prev_space = 1;
            }
        } else {
            if (idx < out_len) out[idx++] = (char)uc;
            started = 1;
            prev_space = 0;
        }
    }

    // Trim trailing space if any
    if (idx > 0 && out[idx - 1] == ' ') {
        idx--;
    }
    out[idx] = '\0';
    return out;
}

int main(int argc, char** argv) {
    // Five test cases
    const char* tests[5] = {
        "Hello,  World!",
        "   Leading and trailing   ",
        "Line1\t\tLine2",
        "Control\aChar and \t tabs",
        "aaaaa     bbbbb     ccccc    "
    };

    for (int i = 0; i < 5; ++i) {
        char* processed = processInput(tests[i]);
        if (processed) {
            printf("Test %d: [%s]\n", i + 1, processed);
            free(processed);
        } else {
            printf("Test %d: [Memory allocation failed]\n", i + 1);
        }
    }

    // If user provides arguments, treat them as user input safely
    if (argc > 1) {
        // Safely join arguments with spaces into a dynamically allocated buffer
        size_t total = 0;
        for (int i = 1; i < argc; ++i) {
            size_t len = strlen(argv[i]);
            if (SIZE_MAX - total < len + 1) { // +1 for space or NUL
                fprintf(stderr, "Input too large.\n");
                return 1;
            }
            total += len + 1;
        }
        char* joined = (char*)malloc(total + 1);
        if (!joined) {
            fprintf(stderr, "Memory allocation failed.\n");
            return 1;
        }
        joined[0] = '\0';
        for (int i = 1; i < argc; ++i) {
            if (i > 1) strcat(joined, " ");
            strcat(joined, argv[i]);
        }

        char* processed = processInput(joined);
        if (processed) {
            printf("User Input: [%s]\n", processed);
            free(processed);
        } else {
            fprintf(stderr, "Processing failed.\n");
        }
        free(joined);
    }

    return 0;
}