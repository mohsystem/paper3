#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* expandedForm(unsigned long long n) {
    if (n == 0ULL) {
        char* res0 = (char*)malloc(2);
        if (res0) { res0[0] = '0'; res0[1] = '\0'; }
        return res0;
    }

    char digits[32];
    int written = snprintf(digits, sizeof(digits), "%llu", n);
    if (written <= 0 || written >= (int)sizeof(digits)) {
        return NULL;
    }

    size_t k = (size_t)written;
    size_t terms = 0;
    size_t sumDigits = 0;

    for (size_t i = 0; i < k; ++i) {
        char c = digits[i];
        if (c != '0') {
            terms++;
            sumDigits += 1 + (k - i - 1);
        }
    }

    if (terms == 0) {
        char* res0 = (char*)malloc(2);
        if (res0) { res0[0] = '0'; res0[1] = '\0'; }
        return res0;
    }

    size_t sepLen = (terms > 1) ? (terms - 1) * 3 : 0;
    size_t totalLen = sumDigits + sepLen + 1;

    char* res = (char*)malloc(totalLen);
    if (!res) return NULL;

    size_t pos = 0;
    size_t termIndex = 0;
    for (size_t i = 0; i < k; ++i) {
        char c = digits[i];
        if (c != '0') {
            if (termIndex > 0) {
                memcpy(res + pos, " + ", 3);
                pos += 3;
            }
            res[pos++] = c;
            size_t zeros = k - i - 1;
            if (zeros > 0) {
                memset(res + pos, '0', zeros);
                pos += zeros;
            }
            termIndex++;
        }
    }
    res[pos] = '\0';
    return res;
}

int main(void) {
    unsigned long long tests[5] = {12ULL, 42ULL, 70304ULL, 9000000ULL, 102030ULL};
    for (int i = 0; i < 5; ++i) {
        char* out = expandedForm(tests[i]);
        if (out) {
            printf("%llu -> %s\n", tests[i], out);
            free(out);
        } else {
            printf("Error processing %llu\n", tests[i]);
        }
    }
    return 0;
}