#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

static const long long LL_MIN_C = LLONG_MIN;
static const long long LL_MAX_C = LLONG_MAX;

static char* make_result_ll(long long v) {
    // "RESULT: " + sign + digits + null
    char buf[64];
    snprintf(buf, sizeof(buf), "RESULT: %lld", v);
    size_t len = strlen(buf);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, buf, len + 1);
    return out;
}

static char* make_error(const char* msg) {
    const char* prefix = "ERROR: ";
    size_t plen = strlen(prefix);
    size_t mlen = strlen(msg);
    char* out = (char*)malloc(plen + mlen + 1);
    if (!out) return NULL;
    memcpy(out, prefix, plen);
    memcpy(out + plen, msg, mlen + 1);
    return out;
}

static void trim(const char* in, const char** start, const char** end) {
    const char* s = in;
    while (*s && isspace((unsigned char)*s)) s++;
    const char* e = in + strlen(in);
    while (e > s && isspace((unsigned char)*(e - 1))) e--;
    *start = s;
    *end = e;
}

static int parse_int64(const char* s, long long* out) {
    // Trim internal spaces are not allowed
    while (*s && isspace((unsigned char)*s)) s++;
    const char* p = s;
    if (*p == '+' || *p == '-') p++;
    if (!*p) return 0;
    while (*p) {
        if (isspace((unsigned char)*p)) return 0;
        if (!isdigit((unsigned char)*p)) return 0;
        p++;
    }
    errno = 0;
    char* endp = NULL;
    long long val = strtoll(s, &endp, 10);
    if (errno != 0 || endp == s || *endp != '\0') return 0;
    *out = val;
    return 1;
}

static int add_checked(long long a, long long b, long long* res) {
    if ((b > 0 && a > LL_MAX_C - b) || (b < 0 && a < LL_MIN_C - b)) return 0;
    *res = a + b;
    return 1;
}

static int sub_checked(long long a, long long b, long long* res) {
    if ((b < 0 && a > LL_MAX_C + b) || (b > 0 && a < LL_MIN_C + b)) return 0;
    *res = a - b;
    return 1;
}

static int mul_checked(long long a, long long b, long long* res) {
    if (a == 0 || b == 0) { *res = 0; return 1; }
    if (a == -1 && b == LL_MIN_C) return 0;
    if (b == -1 && a == LL_MIN_C) return 0;
    if (a > 0) {
        if (b > 0) { if (a > LL_MAX_C / b) return 0; }
        else { if (b < LL_MIN_C / a) return 0; }
    } else {
        if (b > 0) { if (a < LL_MIN_C / b) return 0; }
        else {
            if (a != 0 && b < LL_MAX_C / a) return 0;
        }
    }
    *res = a * b;
    return 1;
}

static int div_checked(long long a, long long b, long long* res) {
    if (b == 0) return 0;
    if (a == LL_MIN_C && b == -1) return 0;
    *res = a / b;
    return 1;
}

static int mod_checked(long long a, long long b, long long* res) {
    if (b == 0) return 0;
    *res = a % b;
    return 1;
}

static int pow_checked(long long base, long long exp, long long* res) {
    if (exp < 0) return 0;
    if (base == 0 && exp == 0) return 0;
    long long result = 1;
    long long b = base;
    long long e = exp;
    while (e > 0) {
        if (e & 1LL) {
            if (!mul_checked(result, b, &result)) return 0;
        }
        e >>= 1LL;
        if (e > 0) {
            if (!mul_checked(b, b, &b)) return 0;
        }
    }
    *res = result;
    return 1;
}

char* process_input(const char* input) {
    if (input == NULL) {
        return make_error("Empty input");
    }
    const char* start;
    const char* end;
    trim(input, &start, &end);
    if (start >= end) {
        return make_error("Empty input");
    }

    // Copy trimmed into buffer
    size_t len = (size_t)(end - start);
    char* buf = (char*)malloc(len + 1);
    if (!buf) return make_error("Unexpected error");
    memcpy(buf, start, len);
    buf[len] = '\0';

    // Tokenize expecting 3 tokens
    char* saveptr = NULL;
    char* token = strtok_r(buf, " \t\r\n", &saveptr);
    char* t1 = token;
    char* t2 = strtok_r(NULL, " \t\r\n", &saveptr);
    char* t3 = strtok_r(NULL, " \t\r\n", &saveptr);
    char* extra = strtok_r(NULL, " \t\r\n", &saveptr);

    if (t1 == NULL || t2 == NULL || t3 == NULL || extra != NULL) {
        free(buf);
        return make_error("Expected format: OP A B");
    }

    // Uppercase op
    for (char* p = t1; *p; ++p) *p = (char)toupper((unsigned char)*p);

    long long a, b;
    if (!parse_int64(t2, &a) || !parse_int64(t3, &b)) {
        free(buf);
        return make_error("Invalid number");
    }

    long long res;
    char* out = NULL;
    if (strcmp(t1, "ADD") == 0) {
        if (!add_checked(a, b, &res)) out = make_error("Overflow");
        else out = make_result_ll(res);
    } else if (strcmp(t1, "SUB") == 0) {
        if (!sub_checked(a, b, &res)) out = make_error("Overflow");
        else out = make_result_ll(res);
    } else if (strcmp(t1, "MUL") == 0) {
        if (!mul_checked(a, b, &res)) out = make_error("Overflow");
        else out = make_result_ll(res);
    } else if (strcmp(t1, "DIV") == 0) {
        if (b == 0) out = make_error("Division by zero");
        else if (!div_checked(a, b, &res)) out = make_error("Overflow");
        else out = make_result_ll(res);
    } else if (strcmp(t1, "MOD") == 0) {
        if (b == 0) out = make_error("Modulus by zero");
        else if (!mod_checked(a, b, &res)) out = make_error("Unexpected error");
        else out = make_result_ll(res);
    } else if (strcmp(t1, "POW") == 0) {
        if (b < 0) out = make_error("Negative exponent");
        else if (a == 0 && b == 0) out = make_error("Undefined 0^0");
        else if (!pow_checked(a, b, &res)) out = make_error("Overflow");
        else out = make_result_ll(res);
    } else {
        out = make_error("Unknown operation");
    }

    free(buf);
    if (!out) out = make_error("Unexpected error");
    return out;
}

int main(void) {
    const char* tests[5] = {
        "ADD 3 5",
        "DIV 10 0",
        "POW 2 10",
        "MUL 9223372036854775807 2",
        "SUB a 5"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = process_input(tests[i]);
        if (res) {
            printf("Input: %s -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: %s -> ERROR: Unexpected error\n", tests[i]);
        }
    }
    return 0;
}