#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

typedef struct {
    int ok;
    long long value;
} ParseResult;

static void trim_bounds(const char* s, const char** out_start, const char** out_end) {
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    *out_start = start;
    *out_end = end;
}

ParseResult try_parse_long_long(const char* input) {
    if (input == NULL) {
        ParseResult r = {0, 0};
        return r;
    }

    const char* start = NULL;
    const char* end = NULL;
    trim_bounds(input, &start, &end);
    if (start == end) {
        ParseResult r = {0, 0};
        return r;
    }

    size_t len = (size_t)(end - start);
    char* buf = (char*)malloc(len + 1);
    if (!buf) {
        ParseResult r = {0, 0};
        return r;
    }
    memcpy(buf, start, len);
    buf[len] = '\0';

    errno = 0;
    char* parse_end = NULL;
    long long val = strtoll(buf, &parse_end, 10);

    int ok = 1;
    if (parse_end == buf || *parse_end != '\0' || errno == ERANGE) {
        ok = 0;
    }

    free(buf);
    ParseResult r = {ok, ok ? val : 0};
    return r;
}

int main(void) {
    char line[4096];
    if (fgets(line, sizeof(line), stdin) != NULL) {
        // Remove trailing newline if present
        size_t n = strlen(line);
        if (n > 0 && line[n - 1] == '\n') {
            line[n - 1] = '\0';
        }
        ParseResult r = try_parse_long_long(line);
        if (r.ok) {
            printf("Parsed integer: %lld\n", r.value);
        } else {
            printf("Invalid integer input.\n");
        }
    } else {
        printf("No input provided.\n");
    }

    const char* tests[5] = {
        "123",
        "   -456   ",
        "+789",
        "9223372036854775808",
        "abc123"
    };
    for (int i = 0; i < 5; ++i) {
        ParseResult r = try_parse_long_long(tests[i]);
        if (r.ok) {
            printf("Test \"%s\" -> %lld\n", tests[i], r.value);
        } else {
            printf("Test \"%s\" -> invalid\n", tests[i]);
        }
    }

    return 0;
}