#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int ok;           /* 1 if success, 0 if overflow */
    long long value;  /* result if ok == 1 */
} AddResult;

static AddResult add_thousand_ll(long long x) {
    const long long K = 1000LL;
    AddResult res;
    if (x > LLONG_MAX - K || x < LLONG_MIN + K) {
        res.ok = 0;
        res.value = 0;
        return res;
    }
    res.ok = 1;
    res.value = x + K;
    return res;
}

static char* trim_inplace(char* s) {
    if (!s) return s;
    char* end;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

static int parse_ll(const char* str, long long* out) {
    if (!str || !out) return 0;
    errno = 0;
    char* dup = strdup(str);
    if (!dup) return 0;
    char* s = trim_inplace(dup);
    if (*s == '\0') {
        free(dup);
        return 0;
    }
    char* endptr = NULL;
    long long val = strtoll(s, &endptr, 10);
    if (errno == ERANGE) {
        free(dup);
        return 0;
    }
    if (endptr == s || *trim_inplace(endptr) != '\0') {
        free(dup);
        return 0;
    }
    *out = val;
    free(dup);
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        long long n;
        if (!parse_ll(argv[1], &n)) {
            fprintf(stderr, "Error: Input must be a valid integer.\n");
            return 1;
        }
        AddResult r = add_thousand_ll(n);
        if (!r.ok) {
            fprintf(stderr, "Error: Integer overflow.\n");
            return 1;
        }
        printf("%lld\n", r.value);
    } else {
        // 5 test cases
        long long tests[5] = {
            0LL,
            1LL,
            -1000LL,
            LLONG_MAX - 1000LL,
            LLONG_MAX
        };
        for (int i = 0; i < 5; ++i) {
            AddResult r = add_thousand_ll(tests[i]);
            if (r.ok) {
                printf("add_thousand_ll(%lld) = %lld\n", tests[i], r.value);
            } else {
                printf("add_thousand_ll(%lld) = error: overflow\n", tests[i]);
            }
        }
    }
    return 0;
}