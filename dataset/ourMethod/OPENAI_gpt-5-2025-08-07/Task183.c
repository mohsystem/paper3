#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct {
    int ok; /* 1 = success, 0 = error */
    unsigned long long value;
} ProductResult;

#define MAX_LEN 1000000
#define MAX_SAFE_SPAN 20 /* ensures 9^span fits within unsigned long long */

static int is_all_digits(const char* s) {
    if (s == NULL) return 0;
    for (const char* p = s; *p; ++p) {
        if (*p < '0' || *p > '9') return 0;
    }
    return 1;
}

ProductResult largest_product(const char* digits, int span) {
    ProductResult err = {0, 0ULL};
    if (digits == NULL) return err;
    size_t n = strlen(digits);
    if (n > MAX_LEN) return err;
    if (span < 0 || (size_t)span > n) return err;
    if (!is_all_digits(digits)) return err;

    if (span == 0) {
        ProductResult ok = {1, 1ULL};
        return ok;
    }
    if (span == 1) {
        unsigned long long maxDigit = 0ULL;
        for (size_t i = 0; i < n; ++i) {
            unsigned long long d = (unsigned long long)(digits[i] - '0');
            if (d > maxDigit) maxDigit = d;
        }
        ProductResult ok = {1, maxDigit};
        return ok;
    }
    if (span > MAX_SAFE_SPAN) return err;

    unsigned long long maxProd = 0ULL;
    for (size_t i = 0; i + (size_t)span <= n; ++i) {
        unsigned long long prod = 1ULL;
        int zeroFound = 0;
        for (int j = 0; j < span; ++j) {
            unsigned int d = (unsigned int)(digits[i + j] - '0');
            if (d == 0U) {
                prod = 0ULL;
                zeroFound = 1;
                break;
            }
            prod *= (unsigned long long)d;
        }
        (void)zeroFound;
        if (prod > maxProd) maxProd = prod;
    }
    ProductResult ok = {1, maxProd};
    return ok;
}

static void run_test(const char* digits, int span) {
    ProductResult r = largest_product(digits, span);
    if (r.ok) {
        printf("%llu\n", r.value);
    } else {
        printf("ERROR\n");
    }
}

int main(void) {
    /* Test 1: Example */
    run_test("63915", 3);

    /* Test 2: Contains zeros */
    run_test("10203", 2);

    /* Test 3: Long sequence with span 6 */
    const char* longSeq = "73167176531330624919225119674426574742355349194934";
    run_test(longSeq, 6);

    /* Test 4: Span 0 */
    run_test("12345", 0);

    /* Test 5: 25 nines, span 21 (should error due to overflow guard) */
    char manyNines[26];
    for (int i = 0; i < 25; ++i) manyNines[i] = '9';
    manyNines[25] = '\0';
    run_test(manyNines, 21);

    return 0;
}