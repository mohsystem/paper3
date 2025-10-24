/* Chain-of-Through process:
 * 1) Problem understanding: Compute factorial of non-negative integer, return as string.
 * 2) Security requirements: Validate input; avoid overflow via base-1e9 big integer; safe memory handling.
 * 3) Secure coding generation: Iterative multiplication using 64-bit intermediates; dynamic growth with checks.
 * 4) Code review: Capacity checks, overflow-aware sizing, NULL checks.
 * 5) Secure code output: Final code with 5 test cases, freeing all allocated memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define BASE 1000000000U /* 1e9 */

static char* dup_literal(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

static int ensure_capacity(uint32_t** arr, size_t* cap, size_t needed) {
    if (*cap >= needed) return 1;
    size_t newcap = *cap ? *cap : 16;
    while (newcap < needed) {
        if (newcap > SIZE_MAX / 2) { newcap = needed; break; }
        newcap *= 2;
    }
    if (newcap > SIZE_MAX / sizeof(uint32_t)) return 0;
    uint32_t* tmp = (uint32_t*)realloc(*arr, newcap * sizeof(uint32_t));
    if (!tmp) return 0;
    *arr = tmp;
    *cap = newcap;
    return 1;
}

char* compute_factorial(int n) {
    if (n < 0) {
        return dup_literal("INVALID");
    }
    uint32_t* limbs = NULL;
    size_t size = 0, cap = 0;

    if (!ensure_capacity(&limbs, &cap, 1)) { free(limbs); return dup_literal("INVALID"); }
    limbs[0] = 1U;
    size = 1;

    for (int i = 2; i <= n; ++i) {
        unsigned long long carry = 0ULL;
        for (size_t j = 0; j < size; ++j) {
            unsigned long long prod = (unsigned long long)limbs[j] * (unsigned long long)i + carry;
            limbs[j] = (uint32_t)(prod % BASE);
            carry = prod / BASE;
        }
        while (carry > 0ULL) {
            if (!ensure_capacity(&limbs, &cap, size + 1)) { free(limbs); return dup_literal("INVALID"); }
            limbs[size++] = (uint32_t)(carry % BASE);
            carry /= BASE;
        }
    }

    if (size == 0) {
        free(limbs);
        return dup_literal("0");
    }

    /* Determine length needed: digits of most significant limb + 9 per remaining limb + 1 for '\0' */
    uint32_t ms = limbs[size - 1];
    int ms_digits = 1;
    uint32_t tmp = ms;
    while (tmp >= 10U) { tmp /= 10U; ms_digits++; }
    /* Safe check for multiplication overflow on size_t */
    unsigned long long total_digits_ull = (unsigned long long)ms_digits + 9ULL * (unsigned long long)(size - 1) + 1ULL;
    if (total_digits_ull > SIZE_MAX) { free(limbs); return dup_literal("INVALID"); }
    size_t total_len = (size_t)total_digits_ull;

    char* out = (char*)malloc(total_len);
    if (!out) { free(limbs); return dup_literal("INVALID"); }

    char* p = out;
    /* write most significant limb */
    int written = snprintf(p, total_len, "%u", ms);
    if (written < 0) { free(limbs); free(out); return dup_literal("INVALID"); }
    size_t used = (size_t)written;

    for (ssize_t i = (ssize_t)size - 2; i >= 0; --i) {
        if (used + 9 >= total_len) { free(limbs); free(out); return dup_literal("INVALID"); }
        /* pad to 9 digits */
        snprintf(out + used, total_len - used, "%09u", limbs[i]);
        used += 9;
    }

    free(limbs);
    return out;
}

int main(void) {
    int tests[5] = {0, 1, 5, 10, 20};
    for (int i = 0; i < 5; ++i) {
        char* res = compute_factorial(tests[i]);
        if (!res) {
            printf("factorial(%d) = INVALID\n", tests[i]);
        } else {
            printf("factorial(%d) = %s\n", tests[i], res);
            free(res);
        }
    }
    return 0;
}