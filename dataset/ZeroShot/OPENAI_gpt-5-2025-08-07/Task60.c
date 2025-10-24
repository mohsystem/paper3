#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* expandedForm(unsigned long long n) {
    if (n == 0ULL) {
        char* res = (char*)malloc(1);
        if (!res) return NULL;
        res[0] = '\0';
        return res;
    }
    char num[32];
    if (snprintf(num, sizeof(num), "%llu", n) < 0) {
        return NULL;
    }
    size_t len = strlen(num);
    size_t total = 0;
    size_t nonZeroCount = 0;

    for (size_t i = 0; i < len; ++i) {
        char d = num[i];
        if (d != '0') {
            total += 1 + (len - 1 - i); // digit + zeros
            if (nonZeroCount > 0) total += 3; // " + "
            nonZeroCount++;
        }
    }

    char* res = (char*)malloc(total + 1);
    if (!res) return NULL;

    size_t pos = 0;
    nonZeroCount = 0;
    for (size_t i = 0; i < len; ++i) {
        char d = num[i];
        if (d != '0') {
            if (nonZeroCount > 0) {
                memcpy(res + pos, " + ", 3);
                pos += 3;
            }
            res[pos++] = d;
            size_t zeros = len - 1 - i;
            for (size_t z = 0; z < zeros; ++z) {
                res[pos++] = '0';
            }
            nonZeroCount++;
        }
    }
    res[pos] = '\0';
    return res;
}

int main(void) {
    unsigned long long tests[] = {12ULL, 42ULL, 70304ULL, 900000ULL, 10501ULL};
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char* out = expandedForm(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            fprintf(stderr, "Error generating expanded form.\n");
        }
    }
    return 0;
}