#include <stdio.h>
#include <string.h>
#include <ctype.h>

long long largestProduct(const char* s, int span) {
    if (span < 0) return 0;
    if (span == 0) return 1;
    if (s == NULL) return 0;
    int n = (int)strlen(s);
    if (span > n) return 0;
    for (int i = 0; i < n; ++i) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    long long best = 0;
    for (int i = 0; i + span <= n; ++i) {
        long long prod = 1;
        for (int j = i; j < i + span; ++j) {
            prod *= (long long)(s[j] - '0');
            if (prod == 0) break;
        }
        if (prod > best) best = prod;
    }
    return best;
}

int main() {
    printf("%lld\n", largestProduct("63915", 3));   // 162
    printf("%lld\n", largestProduct("12345", 2));   // 20
    printf("%lld\n", largestProduct("000123", 2));  // 6
    printf("%lld\n", largestProduct("", 0));        // 1
    printf("%lld\n", largestProduct("99999", 5));   // 59049
    return 0;
}