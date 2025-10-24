#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef unsigned __int128 u128;

static bool is_all_digits(const char* s) {
    if (s == NULL) return false;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
    }
    return true;
}

static u128 recompute_product(const char* s, int start, int span) {
    u128 p = 1;
    for (int i = 0; i < span; ++i) {
        p *= (unsigned)(s[start + i] - '0');
    }
    return p;
}

// Returns 0 on invalid input. Valid zero result is indistinguishable from error.
u128 largest_product(const char* input, int span) {
    if (input == NULL) return 0;
    int n = (int)strlen(input);
    if (span < 0) return 0;
    if (span == 0) return (u128)1;
    if (span > n) return 0;
    if (!is_all_digits(input)) return 0;

    int zeros = 0;
    u128 product = 1;
    bool productValid = true;

    for (int i = 0; i < span; ++i) {
        int d = input[i] - '0';
        if (d == 0) { zeros++; productValid = false; }
        else product *= (unsigned)d;
    }

    u128 mx = (zeros == 0) ? product : (u128)0;

    for (int i = span; i < n; ++i) {
        int outgoing = input[i - span] - '0';
        int incoming = input[i] - '0';

        if (outgoing == 0) {
            zeros--;
            productValid = false;
        } else if (zeros == 0 && productValid) {
            product /= (unsigned)outgoing;
        }

        if (incoming == 0) {
            zeros++;
            productValid = false;
        } else if (zeros == 0 && productValid) {
            product *= (unsigned)incoming;
        }

        if (zeros == 0 && !productValid) {
            product = recompute_product(input, i - span + 1, span);
            productValid = true;
        }

        if (zeros == 0 && product > mx) mx = product;
    }

    return mx;
}

static void print_u128(u128 x) {
    if (x == 0) {
        printf("0");
        return;
    }
    char buf[128];
    int idx = 0;
    while (x > 0) {
        unsigned digit = (unsigned)(x % 10);
        buf[idx++] = (char)('0' + digit);
        x /= 10;
    }
    for (int i = idx - 1; i >= 0; --i) putchar(buf[i]);
}

int main(void) {
    const char* inputs[] = { "63915", "123456789", "1020304050", "000", "" };
    int spans[] = { 3, 2, 2, 2, 0 };
    for (int i = 0; i < 5; ++i) {
        u128 res = largest_product(inputs[i], spans[i]);
        printf("Input: %s, Span: %d -> ", inputs[i], spans[i]);
        print_u128(res);
        printf("\n");
    }
    return 0;
}