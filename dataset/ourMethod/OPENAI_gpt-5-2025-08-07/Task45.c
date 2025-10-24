#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

enum { MAX_LINE_LEN = 1024, MAX_TEXT_LEN = 512, MAX_COUNT = 100 };

static void rstrip(char *s) {
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) { s[--n] = '\0'; }
}

static char* lstrip_dup(const char* s) {
    while (*s && isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

static void to_upper_inplace(char* s) {
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

static int is_printable_ascii(char c) {
    return c >= 32 && c <= 126;
}

static char* sanitize_text(const char* in, size_t maxLen) {
    if (!in) return NULL;
    size_t cap = maxLen + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; in[i] && j < maxLen; ++i) {
        if (is_printable_ascii(in[i])) {
            out[j++] = in[i];
        }
    }
    out[j] = '\0';
    return out;
}

static int safe_str_to_ll(const char* s, long long* out) {
    if (!s || !out) return 0;
    errno = 0;
    char* end = NULL;
    long long v = strtoll(s, &end, 10);
    if (errno == ERANGE) return 0;
    while (end && *end && isspace((unsigned char)*end)) end++;
    if (!end || *end != '\0') return 0;
    *out = v;
    return 1;
}

static int safe_str_to_double(const char* s, double* out) {
    if (!s || !out) return 0;
    errno = 0;
    char* end = NULL;
    double v = strtod(s, &end);
    if (errno == ERANGE) return 0;
    while (end && *end && isspace((unsigned char)*end)) end++;
    if (!end || *end != '\0') return 0;
    *out = v;
    return 1;
}

static int safe_add_ll(long long a, long long b, long long* res) {
    if (!res) return 0;
    if ((b > 0 && a > LLONG_MAX - b) || (b < 0 && a < LLONG_MIN - b)) return 0;
    *res = a + b;
    return 1;
}

static int safe_mul_ll(long long a, long long b, long long* res) {
    if (!res) return 0;
    if (a == 0 || b == 0) { *res = 0; return 1; }
    if (a == -1) { if (b == LLONG_MIN) return 0; *res = -b; return 1; }
    if (b == -1) { if (a == LLONG_MIN) return 0; *res = -a; return 1; }
    if (a > 0) {
        if (b > 0) { if (a > LLONG_MAX / b) return 0; }
        else { if (b < LLONG_MIN / a) return 0; }
    } else {
        if (b > 0) { if (a < LLONG_MIN / b) return 0; }
        else { if (a != 0 && b < LLONG_MAX / a) return 0; }
    }
    *res = a * b;
    return 1;
}

static void format_double(double v, char* buf, size_t buflen) {
    if (!buf || buflen == 0) return;
    // print with 6 decimals then trim
    int n = snprintf(buf, buflen, "%.6f", v);
    if (n < 0 || (size_t)n >= buflen) { // truncated; ensure null-termination
        if (buflen > 0) buf[buflen - 1] = '\0';
        return;
    }
    size_t len = (size_t)n;
    // Trim trailing zeros and dot
    size_t pos = 0;
    for (size_t i = 0; i < len; ++i) if (buf[i] == '.') { pos = i; break; }
    if (pos > 0) {
        size_t end = len;
        while (end > pos && buf[end - 1] == '0') --end;
        if (end > pos && buf[end - 1] == '.') --end;
        buf[end] = '\0';
    }
    if (strcmp(buf, "-0") == 0) { snprintf(buf, buflen, "0"); }
}

static int split_ws(const char* s, char*** outTokens, size_t* outCount) {
    if (!s || !outTokens || !outCount) return 0;
    // Make a working copy
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (!copy) return 0;
    memcpy(copy, s, len + 1);
    size_t cap = 8;
    size_t count = 0;
    char** arr = (char**)malloc(cap * sizeof(char*));
    if (!arr) { free(copy); return 0; }
#if defined(_WIN32)
    char* ctx = NULL;
    char* tok = strtok_s(copy, " \t\r\n", &ctx);
#else
    char* ctx = NULL;
    char* tok = strtok_r(copy, " \t\r\n", &ctx);
#endif
    while (tok) {
        if (count == cap) {
            size_t ncap = cap * 2;
            char** narr = (char**)realloc(arr, ncap * sizeof(char*));
            if (!narr) {
                free(arr);
                free(copy);
                return 0;
            }
            arr = narr;
            cap = ncap;
        }
        arr[count++] = tok;
#if defined(_WIN32)
        tok = strtok_s(NULL, " \t\r\n", &ctx);
#else
        tok = strtok_r(NULL, " \t\r\n", &ctx);
#endif
    }
    // We return the array and a separate buffer pointer at index count (hidden).
    // Store the base buffer pointer right after the tokens array for later free.
    // Simpler: just store at outTokens[0] - 1, but that's unsafe. Instead, wrap in a struct, but spec requires single file.
    // We'll return tokens with a hidden header: [buffer_ptr][token_ptrs...]
    char** result = (char**)malloc((count + 1) * sizeof(char*));
    if (!result) {
        free(arr);
        free(copy);
        return 0;
    }
    for (size_t i = 0; i < count; ++i) result[i] = arr[i];
    result[count] = copy; // store buffer to free later
    free(arr);
    *outTokens = result;
    *outCount = count;
    return 1;
}

static void free_tokens(char** tokens, size_t count) {
    if (!tokens) return;
    if (tokens[count]) free(tokens[count]); // free backing buffer
    free(tokens);
}

static char* join_doubles(const double* arr, size_t n) {
    if (!arr) return NULL;
    // Each double formatted up to ~32 chars + space; cap by input size
    size_t cap = 2 * MAX_LINE_LEN + 64;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';
    size_t used = 0;
    for (size_t i = 0; i < n; ++i) {
        char buf[64];
        format_double(arr[i], buf, sizeof(buf));
        size_t need = strlen(buf) + (i ? 1 : 0) + 1;
        if (used + need >= cap) {
            size_t ncap = cap * 2;
            if (ncap > 65536) { // hard ceiling to avoid excessive memory
                free(out);
                return NULL;
            }
            char* tmp = (char*)realloc(out, ncap);
            if (!tmp) { free(out); return NULL; }
            out = tmp; cap = ncap;
        }
        if (i) { out[used++] = ' '; }
        size_t blen = strlen(buf);
        memcpy(out + used, buf, blen);
        used += blen;
        out[used] = '\0';
    }
    return out;
}

static char* process_line(const char* raw) {
    if (!raw) {
        char* e = strdup("ERROR: null input");
        return e ? e : NULL;
    }
    size_t len = strlen(raw);
    if (len > MAX_LINE_LEN) {
        char* e = strdup("ERROR: line too long (max 1024)");
        return e ? e : NULL;
    }
    // Make a working trimmed copy
    char* work = lstrip_dup(raw);
    if (!work) return strdup("ERROR: out of memory");
    rstrip(work);
    if (work[0] == '\0') { free(work); return strdup("ERROR: empty input"); }

    // Extract command
    size_t i = 0;
    while (work[i] && !isspace((unsigned char)work[i])) i++;
    char cmd[32];
    size_t cmdlen = i < sizeof(cmd) - 1 ? i : sizeof(cmd) - 1;
    memcpy(cmd, work, cmdlen);
    cmd[cmdlen] = '\0';
    to_upper_inplace(cmd);

    const char* rest = work + i;
    while (*rest && isspace((unsigned char)*rest)) rest++;

    if (strcmp(cmd, "REVERSE") == 0) {
        char* sanitized = sanitize_text(rest, MAX_TEXT_LEN);
        if (!sanitized) { free(work); return strdup("ERROR: out of memory"); }
        size_t slen = strlen(sanitized);
        for (size_t a = 0, b = slen ? slen - 1 : 0; a < b; ++a, --b) {
            char t = sanitized[a]; sanitized[a] = sanitized[b]; sanitized[b] = t;
        }
        free(work);
        return sanitized;
    } else if (strcmp(cmd, "UPPER") == 0) {
        char* sanitized = sanitize_text(rest, MAX_TEXT_LEN);
        if (!sanitized) { free(work); return strdup("ERROR: out of memory"); }
        to_upper_inplace(sanitized);
        free(work);
        return sanitized;
    } else if (strcmp(cmd, "ADD") == 0) {
        char** toks = NULL; size_t cnt = 0;
        if (!split_ws(rest, &toks, &cnt)) { free(work); return strdup("ERROR: out of memory"); }
        if (cnt != 2) { free_tokens(toks, cnt); free(work); return strdup("ERROR: ADD expects 2 integers"); }
        long long a = 0, b = 0, r = 0;
        int ok = safe_str_to_ll(toks[0], &a) && safe_str_to_ll(toks[1], &b);
        if (!ok) { free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid integer"); }
        if (!safe_add_ll(a, b, &r)) { free_tokens(toks, cnt); free(work); return strdup("ERROR: integer overflow on addition"); }
        char buf[64]; snprintf(buf, sizeof(buf), "%lld", r);
        char* out = strdup(buf);
        free_tokens(toks, cnt);
        free(work);
        return out ? out : strdup("ERROR: out of memory");
    } else if (strcmp(cmd, "MUL") == 0) {
        char** toks = NULL; size_t cnt = 0;
        if (!split_ws(rest, &toks, &cnt)) { free(work); return strdup("ERROR: out of memory"); }
        if (cnt != 2) { free_tokens(toks, cnt); free(work); return strdup("ERROR: MUL expects 2 integers"); }
        long long a = 0, b = 0, r = 0;
        int ok = safe_str_to_ll(toks[0], &a) && safe_str_to_ll(toks[1], &b);
        if (!ok) { free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid integer"); }
        if (!safe_mul_ll(a, b, &r)) { free_tokens(toks, cnt); free(work); return strdup("ERROR: integer overflow on multiplication"); }
        char buf[64]; snprintf(buf, sizeof(buf), "%lld", r);
        char* out = strdup(buf);
        free_tokens(toks, cnt);
        free(work);
        return out ? out : strdup("ERROR: out of memory");
    } else if (strcmp(cmd, "AVG") == 0) {
        char** toks = NULL; size_t cnt = 0;
        if (!split_ws(rest, &toks, &cnt)) { free(work); return strdup("ERROR: out of memory"); }
        if (cnt < 2) { free_tokens(toks, cnt); free(work); return strdup("ERROR: AVG expects count followed by numbers"); }
        long long count = 0;
        if (!safe_str_to_ll(toks[0], &count) || count < 1 || count > MAX_COUNT) {
            free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid count (1..100)");
        }
        if ((size_t)count + 1 != cnt) { free_tokens(toks, cnt); free(work); return strdup("ERROR: count does not match number of values"); }
        long double sum = 0.0L;
        for (long long i2 = 0; i2 < count; ++i2) {
            double v = 0.0;
            if (!safe_str_to_double(toks[i2 + 1], &v)) { free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid number"); }
            sum += v;
        }
        double avg = (double)(sum / (long double)count);
        char bufd[64]; format_double(avg, bufd, sizeof(bufd));
        char* out = strdup(bufd);
        free_tokens(toks, cnt);
        free(work);
        return out ? out : strdup("ERROR: out of memory");
    } else if (strcmp(cmd, "SORT") == 0) {
        char** toks = NULL; size_t cnt = 0;
        if (!split_ws(rest, &toks, &cnt)) { free(work); return strdup("ERROR: out of memory"); }
        if (cnt < 2) { free_tokens(toks, cnt); free(work); return strdup("ERROR: SORT expects count followed by numbers"); }
        long long count = 0;
        if (!safe_str_to_ll(toks[0], &count) || count < 1 || count > MAX_COUNT) {
            free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid count (1..100)");
        }
        if ((size_t)count + 1 != cnt) { free_tokens(toks, cnt); free(work); return strdup("ERROR: count does not match number of values"); }
        double* arr = (double*)calloc((size_t)count, sizeof(double));
        if (!arr) { free_tokens(toks, cnt); free(work); return strdup("ERROR: out of memory"); }
        for (long long i2 = 0; i2 < count; ++i2) {
            if (!safe_str_to_double(toks[i2 + 1], &arr[i2])) {
                free(arr); free_tokens(toks, cnt); free(work); return strdup("ERROR: invalid number");
            }
        }
        // sort
        int cmpd(const void* a, const void* b) {
            double da = *(const double*)a, db = *(const double*)b;
            return (da > db) - (da < db);
        }
        qsort(arr, (size_t)count, sizeof(double), cmpd);
        char* out = join_doubles(arr, (size_t)count);
        free(arr);
        free_tokens(toks, cnt);
        free(work);
        if (!out) return strdup("ERROR: out of memory");
        return out;
    } else {
        free(work);
        return strdup("ERROR: unknown command");
    }
}

int main(void) {
    // 5 test cases
    const char* tests[5] = {
        "ADD 10 20",
        "MUL 2000000000 3",
        "REVERSE Hello, World!",
        "AVG 5 1 2 3 4 5",
        "SORT 5 5 1 4 3 2"
    };
    printf("Test cases:\n");
    for (int i = 0; i < 5; ++i) {
        char* out = process_line(tests[i]);
        if (out) {
            printf("%s => %s\n", tests[i], out);
            free(out);
        } else {
            printf("%s => ERROR: out of memory\n", tests[i]);
        }
    }

    printf("\nEnter commands (CTRL+D to end):\n");
    char buf[MAX_LINE_LEN + 3];
    while (fgets(buf, (int)sizeof(buf), stdin) != NULL) {
        size_t len = strlen(buf);
        if (len == MAX_LINE_LEN + 2 && buf[len - 1] != '\n') {
            // Line too long; flush remainder
            int c;
            while ((c = fgetc(stdin)) != EOF && c != '\n') {}
            puts("ERROR: line too long (max 1024)");
            continue;
        }
        if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
        char* out = process_line(buf);
        if (out) {
            puts(out);
            free(out);
        } else {
            puts("ERROR: out of memory");
        }
    }
    return 0;
}