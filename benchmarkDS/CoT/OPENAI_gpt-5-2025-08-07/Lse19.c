/* Step 1: Problem understanding:
   - Read a number from the command line, add 1000, print result.
   - Provide a function that accepts input and returns output (with error flag).
*/

/* Step 2: Security requirements:
   - Strict input parsing and validation.
   - Prevent overflow/underflow in addition.
*/

/* Step 3: Secure coding generation:
   - Use strtoll with endptr and errno.
   - Check boundaries before addition.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/* Strict parser: trims whitespace, parses entire string as long long */
int parse_ll_strict(const char *s, long long *out) {
    if (s == NULL || out == NULL) return 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;

    char *endptr = NULL;
    errno = 0;
    long long val = strtoll(s, &endptr, 10);
    if (errno != 0) return 0;
    if (endptr == s) return 0;
    while (isspace((unsigned char)*endptr)) endptr++;
    if (*endptr != '\0') return 0;

    *out = val;
    return 1;
}

/* Returns result; sets *error to 1 on overflow, 0 otherwise */
long long add1000_ll(long long value, int *error) {
    if (error) *error = 0;
    if (value > LLONG_MAX - 1000LL || value < LLONG_MIN + 1000LL) {
        if (error) *error = 1;
        return 0LL;
    }
    return value + 1000LL;
}

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        long long v = 0;
        if (!parse_ll_strict(argv[1], &v)) {
            fprintf(stderr, "Invalid integer input.\n");
            return 1;
        }
        int err = 0;
        long long res = add1000_ll(v, &err);
        if (err) {
            fprintf(stderr, "Overflow detected.\n");
            return 1;
        }
        printf("%lld\n", res);
    } else {
        long long tests[5] = {0LL, 42LL, -42LL, LLONG_MAX - 1000LL, LLONG_MIN + 1000LL};
        for (int i = 0; i < 5; i++) {
            int err = 0;
            long long res = add1000_ll(tests[i], &err);
            if (err) {
                printf("add1000(%lld) = overflow\n", tests[i]);
            } else {
                printf("add1000(%lld) = %lld\n", tests[i], res);
            }
        }
    }
    return 0;
}