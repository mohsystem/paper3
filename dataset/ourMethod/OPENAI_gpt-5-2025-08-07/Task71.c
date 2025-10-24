#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_INPUT_LEN 128

typedef enum {
    PARSE_OK = 0,
    ERR_NULL = 1,
    ERR_TOO_LONG = 2,
    ERR_EMPTY = 3,
    ERR_INVALID = 4,
    ERR_OVERFLOW = 5
} ParseErr;

typedef struct {
    int ok;            /* 1 on success, 0 on failure */
    long long value;   /* parsed value on success */
    ParseErr err;      /* error code on failure */
} ParseResult;

/* Trim leading and trailing whitespace from 'in' into 'out' with bounds checks. */
static int sanitize_and_trim(const char* in, char* out, size_t outsz) {
    if (in == NULL || out == NULL || outsz == 0) {
        return -1;
    }
    /* Enforce max input length to prevent excessive processing */
    size_t in_len = 0;
    while (in[in_len] != '\0') {
        if (in_len + 1 >= outsz) { /* +1 to ensure room for NUL terminator */
            return ERR_TOO_LONG;
        }
        in_len++;
    }

    /* Find start and end for trimming */
    size_t start = 0;
    while (start < in_len && isspace((unsigned char)in[start])) {
        start++;
    }
    if (start == in_len) {
        /* All whitespace -> empty after trim */
        out[0] = '\0';
        return 0;
    }
    size_t end = in_len - 1;
    while (end > start && isspace((unsigned char)in[end])) {
        end--;
    }

    size_t trimmed_len = end - start + 1;
    if (trimmed_len + 1 > outsz) {
        return ERR_TOO_LONG;
    }
    memcpy(out, in + start, trimmed_len);
    out[trimmed_len] = '\0';
    return 0;
}

static const char* parse_err_str(ParseErr e) {
    switch (e) {
        case PARSE_OK: return "no error";
        case ERR_NULL: return "null input";
        case ERR_TOO_LONG: return "input too long";
        case ERR_EMPTY: return "empty input";
        case ERR_INVALID: return "invalid integer format";
        case ERR_OVERFLOW: return "integer overflow";
        default: return "unknown error";
    }
}

/* Parse a string to long long with strict validation (no trailing chars, base 10). */
static ParseResult parse_integer(const char* input) {
    ParseResult r;
    r.ok = 0;
    r.value = 0;
    r.err = PARSE_OK;

    if (input == NULL) {
        r.err = ERR_NULL;
        return r;
    }

    char buf[MAX_INPUT_LEN];
    int sres = sanitize_and_trim(input, buf, sizeof(buf));
    if (sres == ERR_TOO_LONG) {
        r.err = ERR_TOO_LONG;
        return r;
    } else if (sres != 0) {
        r.err = ERR_INVALID;
        return r;
    }

    if (buf[0] == '\0') {
        r.err = ERR_EMPTY;
        return r;
    }

    errno = 0;
    char* endptr = NULL;
    long long val = strtoll(buf, &endptr, 10);

    if (buf == endptr) {
        r.err = ERR_INVALID; /* no digits consumed */
        return r;
    }
    if (errno == ERANGE) {
        r.err = ERR_OVERFLOW;
        return r;
    }
    if (endptr == NULL || *endptr != '\0') {
        r.err = ERR_INVALID; /* trailing invalid characters */
        return r;
    }

    r.ok = 1;
    r.value = val;
    r.err = PARSE_OK;
    return r;
}

static void run_and_print(const char* s) {
    ParseResult res = parse_integer(s);
    if (s == NULL) {
        s = "(null)";
    }
    if (res.ok) {
        /* Use a constant format string */
        printf("Input: \"%s\" -> OK, value=%lld\n", s, res.value);
    } else {
        printf("Input: \"%s\" -> ERROR, %s\n", s, parse_err_str(res.err));
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            run_and_print(argv[i]);
        }
    } else {
        /* 5 test cases */
        const char* tests[5] = {
            "123",
            "-42",
            "  17",
            "9223372036854775808", /* overflow for 64-bit */
            "12abc"
        };
        for (int i = 0; i < 5; ++i) {
            run_and_print(tests[i]);
        }
    }
    return 0;
}