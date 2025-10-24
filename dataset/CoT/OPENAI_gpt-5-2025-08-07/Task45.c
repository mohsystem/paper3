/* Chain-of-Through process:
1) Problem understanding: Parse "OP A B", execute 64-bit safe arithmetic, return "Result: <value>" or "Error: <reason>".
2) Security requirements: input length checks, strict tokenization, safe parsing with strtoll, overflow-checked arithmetic.
3) Secure coding generation: helper functions for safe add/sub/mul/div, no undefined behavior.
4) Code review: Verified overflow checks, division by zero, allocation sizing, errors.
5) Secure code output: Final C implementation below.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

static int safe_add_ll(long long a, long long b, long long* out) {
    if (b > 0 && a > LLONG_MAX - b) return 0;
    if (b < 0 && a < LLONG_MIN - b) return 0;
    *out = a + b;
    return 1;
}

static int safe_sub_ll(long long a, long long b, long long* out) {
    if (b > 0 && a < LLONG_MIN + b) return 0;
    if (b < 0 && a > LLONG_MAX + b) return 0;
    *out = a - b;
    return 1;
}

static int safe_mul_ll(long long a, long long b, long long* out) {
    if (a == 0 || b == 0) { *out = 0; return 1; }
    if (a == -1 && b == LLONG_MIN) return 0;
    if (b == -1 && a == LLONG_MIN) return 0;

    if (a > 0) {
        if (b > 0) {
            if (a > LLONG_MAX / b) return 0;
        } else { // b < 0
            if (b < LLONG_MIN / a) return 0;
        }
    } else { // a < 0
        if (b > 0) {
            if (a < LLONG_MIN / b) return 0;
        } else { // b < 0
            if (a != 0 && b < LLONG_MAX / a) return 0;
        }
    }
    *out = a * b;
    return 1;
}

static int safe_div_ll(long long a, long long b, long long* out) {
    if (b == 0) return 0;
    if (a == LLONG_MIN && b == -1) return 0;
    *out = a / b;
    return 1;
}

static int parse_int64_strict(const char* s, long long* out) {
    if (s == NULL || *s == '\0') return 0;
    // Ensure only [+,-,digits]
    for (const char* p = s; *p; ++p) {
        if (!(*p == '+' || *p == '-' || (*p >= '0' && *p <= '9'))) return 0;
    }
    errno = 0;
    char* endptr = NULL;
    long long val = strtoll(s, &endptr, 10);
    if (errno == ERANGE) return 0;
    if (endptr == s || *endptr != '\0') return 0;
    *out = val;
    return 1;
}

static char* make_message(const char* prefix, long long value, int is_result) {
    // Allocate sufficient buffer
    // Max length of signed 64-bit number string is 20 chars plus sign, so 21; prefix up to ~10 chars
    char buf[64];
    if (is_result) {
        // "Result: <value>"
        // Use snprintf to generate into a local buffer first
        snprintf(buf, sizeof(buf), "%s %lld", prefix, value);
    } else {
        // "Error: <text>" - prefix already full text
        snprintf(buf, sizeof(buf), "%s", prefix);
    }
    size_t len = strnlen(buf, sizeof(buf));
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL; // allocation failure
    memcpy(out, buf, len);
    out[len] = '\0';
    return out;
}

char* process_input(const char* input) {
    if (input == NULL) {
        char* msg = (char*)malloc(24);
        if (!msg) return NULL;
        strcpy(msg, "Error: input is null");
        return msg;
    }
    // Trim leading/trailing whitespace safely
    while (isspace((unsigned char)*input)) input++;
    size_t inlen = strnlen(input, 2001);
    if (inlen == 0) {
        char* msg = (char*)malloc(22);
        if (!msg) return NULL;
        strcpy(msg, "Error: input is empty");
        return msg;
    }
    if (inlen > 1000) {
        char* msg = (char*)malloc(23);
        if (!msg) return NULL;
        strcpy(msg, "Error: input too long");
        return msg;
    }

    // Copy to a local buffer to tokenize
    char buf[1024 + 1];
    memcpy(buf, input, inlen);
    buf[inlen] = '\0';
    // Right trim
    for (ssize_t i = (ssize_t)inlen - 1; i >= 0; --i) {
        if (!isspace((unsigned char)buf[i])) break;
        buf[i] = '\0';
    }

    // Tokenize into exactly 3 tokens
    char* tokens[4] = {0};
    int count = 0;
    char* saveptr = NULL;
    char* tok = strtok_r(buf, " \t\r\n\v\f", &saveptr);
    while (tok != NULL && count < 4) {
        tokens[count++] = tok;
        tok = strtok_r(NULL, " \t\r\n\v\f", &saveptr);
    }
    if (count != 3) {
        char* msg = (char*)malloc(61);
        if (!msg) return NULL;
        strcpy(msg, "Error: expected format 'OP A B' with exactly three tokens");
        return msg;
    }

    // Uppercase OP
    for (char* p = tokens[0]; *p; ++p) *p = (char)toupper((unsigned char)*p);

    long long a, b;
    if (!parse_int64_strict(tokens[1], &a)) {
        char* msg = (char*)malloc(26);
        if (!msg) return NULL;
        strcpy(msg, "Error: invalid number for A");
        return msg;
    }
    if (!parse_int64_strict(tokens[2], &b)) {
        char* msg = (char*)malloc(26);
        if (!msg) return NULL;
        strcpy(msg, "Error: invalid number for B");
        return msg;
    }

    long long res;
    if (strcmp(tokens[0], "ADD") == 0) {
        if (!safe_add_ll(a, b, &res)) {
            char* msg = (char*)malloc(29);
            if (!msg) return NULL;
            strcpy(msg, "Error: overflow in addition");
            return msg;
        }
        return make_message("Result:", res, 1);
    } else if (strcmp(tokens[0], "SUB") == 0) {
        if (!safe_sub_ll(a, b, &res)) {
            char* msg = (char*)malloc(31);
            if (!msg) return NULL;
            strcpy(msg, "Error: overflow in subtraction");
            return msg;
        }
        return make_message("Result:", res, 1);
    } else if (strcmp(tokens[0], "MUL") == 0) {
        if (!safe_mul_ll(a, b, &res)) {
            char* msg = (char*)malloc(35);
            if (!msg) return NULL;
            strcpy(msg, "Error: overflow in multiplication");
            return msg;
        }
        return make_message("Result:", res, 1);
    } else if (strcmp(tokens[0], "DIV") == 0) {
        if (!safe_div_ll(a, b, &res)) {
            char* msg = (char*)malloc(49);
            if (!msg) return NULL;
            strcpy(msg, "Error: division error (division by zero or overflow)");
            return msg;
        }
        return make_message("Result:", res, 1);
    } else {
        char* msg = (char*)malloc(29);
        if (!msg) return NULL;
        strcpy(msg, "Error: unsupported operation");
        return msg;
    }
}

int main(void) {
    const char* tests[5] = {
        "ADD 5 7",
        "DIV 10 0",
        "MUL 9223372036854775807 2",
        "SUB -5 -10",
        "FOO 1 2"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = process_input(tests[i]);
        if (res) {
            printf("Input: \"%s\" -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: \"%s\" -> Error: allocation failure\n", tests[i]);
        }
    }
    return 0;
}