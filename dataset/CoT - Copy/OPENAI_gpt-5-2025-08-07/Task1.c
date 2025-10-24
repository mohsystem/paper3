/* Chain-of-Through Process:
 * 1) Problem understanding: compute remaining people on the bus after (on, off) pairs.
 * 2) Security requirements: validate inputs (non-negative, off not exceeding available).
 * 3) Secure coding generation: use long long for accumulation, check bounds.
 * 4) Code review: ensure no negative totals and safe return.
 * 5) Secure code output: function returns non-negative count or -1 on invalid input.
 */
#include <stdio.h>
#include <stddef.h>
#include <limits.h>

int countPassengers(const int stops[][2], size_t n) {
    long long total = 0LL;
    for (size_t i = 0; i < n; ++i) {
        int on = stops[i][0];
        int off = stops[i][1];
        if (on < 0 || off < 0) {
            return -1; // invalid input
        }
        long long available = total + (long long)on;
        if ((long long)off > available) {
            return -1; // invalid transition
        }
        total = available - (long long)off;
        if (total < 0) {
            return -1;
        }
        if (total > INT_MAX) {
            return -1; // overflow protection for return type
        }
    }
    return (int)total;
}

int main(void) {
    const int t1[][2] = { {10,0}, {3,5}, {5,8} }; /* expected 5 */
    const int t2[][2] = { }; /* expected 0 */
    const int t3[][2] = { {0,0} }; /* expected 0 */
    const int t4[][2] = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} }; /* expected 17 */
    const int t5[][2] = { {5,0}, {0,5} }; /* expected 0 */

    printf("%d\n", countPassengers(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%d\n", countPassengers(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%d\n", countPassengers(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%d\n", countPassengers(t4, sizeof(t4)/sizeof(t4[0])));
    printf("%d\n", countPassengers(t5, sizeof(t5)/sizeof(t5[0])));
    return 0;
}