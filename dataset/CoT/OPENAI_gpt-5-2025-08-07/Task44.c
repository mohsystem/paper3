#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

static int is_allowed_char(char c) {
    return (isdigit((unsigned char)c) || c == ' ' || c == '\t' || c == '\n' || c == ',' || c == '+' || c == '-');
}

static int is_delim(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == ',');
}

char* processInput(const char* input) {
    if (input == NULL) {
        char* r = (char*)malloc(24);
        if (r) strcpy(r, "ERROR: input is null");
        return r;
    }
    size_t len = strlen(input);
    if (len == 0) {
        char* r = (char*)malloc(20);
        if (r) strcpy(r, "ERROR: empty input");
        return r;
    }
    if (len > 1000) {
        char* r = (char*)malloc(23);
        if (r) strcpy(r, "ERROR: input too long");
        return r;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!is_allowed_char(input[i])) {
            char* r = (char*)malloc(33);
            if (r) strcpy(r, "ERROR: invalid characters present");
            return r;
        }
    }

    long long nums[100];
    int count = 0;

    // Tokenize manually
    char token[64];
    int tok_len = 0;
    int any = 0;
    for (size_t i = 0; i <= len; ++i) {
        char c = (i < len) ? input[i] : ' '; // force flush at end
        if (!is_delim(c) && i < len) {
            if (tok_len < (int)sizeof(token) - 1) {
                token[tok_len++] = c;
            } else {
                char* r = (char*)malloc(33);
                if (r) strcpy(r, "ERROR: invalid number: too long");
                return r;
            }
        } else {
            if (tok_len > 0) {
                token[tok_len] = '\0';
                any = 1;

                if (count >= 100) {
                    char* r = (char*)malloc(36);
                    if (r) strcpy(r, "ERROR: too many numbers (limit 100)");
                    return r;
                }

                // Validate numeric: optional sign + digits
                int start = 0;
                if (token[0] == '+' || token[0] == '-') start = 1;
                int digits = 0;
                for (int j = start; token[j] != '\0'; ++j) {
                    if (!isdigit((unsigned char)token[j])) {
                        char* r = (char*)malloc(26 + tok_len);
                        if (r) {
                            strcpy(r, "ERROR: invalid number: ");
                            strcat(r, token);
                        }
                        return r;
                    }
                    digits++;
                }
                if (digits == 0) {
                    char* r = (char*)malloc(26 + tok_len);
                    if (r) {
                        strcpy(r, "ERROR: invalid number: ");
                        strcat(r, token);
                    }
                    return r;
                }
                if (digits > 19) {
                    char* r = (char*)malloc(32 + tok_len);
                    if (r) {
                        strcpy(r, "ERROR: number out of range: ");
                        strcat(r, token);
                    }
                    return r;
                }

                errno = 0;
                char* endp = NULL;
                long long val = strtoll(token, &endp, 10);
                if (errno == ERANGE || endp == token || *endp != '\0') {
                    char* r = (char*)malloc(32 + tok_len);
                    if (r) {
                        strcpy(r, "ERROR: number out of range: ");
                        strcat(r, token);
                    }
                    return r;
                }
                nums[count++] = val;
                tok_len = 0;
            }
        }
    }

    if (!any || count == 0) {
        char* r = (char*)malloc(26);
        if (r) strcpy(r, "ERROR: no numbers found");
        return r;
    }

    long long sum = 0;
    long long mn = LLONG_MAX;
    long long mx = LLONG_MIN;
    for (int i = 0; i < count; ++i) {
        long long v = nums[i];
        if (v < mn) mn = v;
        if (v > mx) mx = v;

        if ((v > 0 && sum > LLONG_MAX - v) || (v < 0 && sum < LLONG_MIN - v)) {
            char* r = (char*)malloc(21);
            if (r) strcpy(r, "ERROR: sum overflow");
            return r;
        }
        sum += v;
    }

    long double avg = (long double)sum / (long double)count;

    // Allocate result buffer
    // Max lengths: count(3), sum(20), min(20), max(20), avg(~40)
    // Total conservative: 160 bytes
    char* out = (char*)malloc(200);
    if (!out) return NULL;

    // Format average with reasonable precision and trim trailing zeros
    char avgbuf[64];
    snprintf(avgbuf, sizeof(avgbuf), "%.10Lf", avg);
    // trim trailing zeros
    size_t alen = strlen(avgbuf);
    while (alen > 0 && avgbuf[alen - 1] == '0') { avgbuf[--alen] = '\0'; }
    if (alen > 0 && avgbuf[alen - 1] == '.') { avgbuf[--alen] = '\0'; }

    snprintf(out, 200, "OK: count=%d, sum=%lld, min=%lld, max=%lld, average=%s",
             count, sum, mn, mx, avgbuf);

    return out;
}

int main(void) {
    const char* tests[5] = {
        "1, 2, 3, 4, 5",
        " 10 20 30 ",
        "-5, -10, 15",
        "abc",
        "9223372036854775807, 1"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = processInput(tests[i]);
        if (res) {
            printf("Input: \"%s\" -> %s\n", tests[i], res);
            free(res);
        } else {
            printf("Input: \"%s\" -> ERROR: allocation failed\n", tests[i]);
        }
    }
    return 0;
}