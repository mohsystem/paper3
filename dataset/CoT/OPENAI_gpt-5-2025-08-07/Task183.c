/* Task183 - C implementation
   Chain-of-Through secure coding process:
   1) Problem understanding: compute largest product of contiguous series in a digit string given span.
   2) Security requirements: validate inputs (NULL pointer, negative span, non-digits, span bounds), avoid UB.
   3) Secure coding generation: simple O(n*span) loops; handle span==0; early break on zero; basic overflow capping.
   4) Code review: ensure bounds checks and safe operations; use size_t for indexes.
   5) Secure code output: include 5 test cases printing results.
*/

#include <stdio.h>
#include <string.h>
#include <limits.h>

unsigned long long largest_product(const char* digits, int span) {
    if (span < 0) return 0ULL;
    if (span == 0) return 1ULL;
    if (digits == NULL) return 0ULL;

    size_t n = strlen(digits);
    if ((size_t)span > n) return 0ULL;

    // Validate digits
    for (size_t i = 0; i < n; ++i) {
        if (digits[i] < '0' || digits[i] > '9') {
            return 0ULL;
        }
    }

    unsigned long long maxProduct = 0ULL;
    for (size_t i = 0; i + (size_t)span <= n; ++i) {
        unsigned long long product = 1ULL;
        int zeroFound = 0;
        for (size_t j = i; j < i + (size_t)span; ++j) {
            unsigned int d = (unsigned int)(digits[j] - '0');
            if (d == 0U) {
                product = 0ULL;
                zeroFound = 1;
                break;
            }
            // Overflow check (best-effort): if product > ULLONG_MAX / d, cap to ULLONG_MAX
            if (product > ULLONG_MAX / d) {
                product = ULLONG_MAX;
                // continue
            } else {
                product *= d;
            }
        }
        if (product > maxProduct) {
            maxProduct = product;
        }
        if (maxProduct == ULLONG_MAX) break;
        (void)zeroFound; // silence unused warning if not used
    }
    return maxProduct;
}

int main(void) {
    // 5 test cases
    const char* inputs[5] = {
        "63915",
        "1234567890",
        "0000",
        "99999",
        ""
    };
    int spans[5] = {3, 2, 2, 5, 0};

    for (int i = 0; i < 5; ++i) {
        unsigned long long result = largest_product(inputs[i], spans[i]);
        printf("Test %d: largest_product(\"%s\", %d) = %llu\n", i + 1, inputs[i], spans[i], (unsigned long long)result);
    }

    return 0;
}