#include <stdio.h>
#include <limits.h>
#include <stddef.h>

typedef struct {
    int ok;              /* 1 for success, 0 for failure */
    long long value;     /* result value if ok==1 */
    const char* error;   /* error message if ok==0 */
} Result;

static int safe_add_ll(long long a, long long b, long long* out) {
    if (b > 0 && a > LLONG_MAX - b) return 0;
    if (b < 0 && a < LLONG_MIN - b) return 0;
    if (out) *out = a + b;
    return 1;
}

Result max_subarray_sum(const int* arr, size_t len) {
    if (arr == NULL || len == 0) {
        Result r = {0, 0LL, "invalid input: null or empty array"};
        return r;
    }

    long long current = (long long)arr[0];
    long long best = current;

    for (size_t i = 1; i < len; i++) {
        long long v = (long long)arr[i];
        long long sum = 0;
        if (!safe_add_ll(current, v, &sum)) {
            Result r = {0, 0LL, "sum overflow detected"};
            return r;
        }
        current = (sum >= v) ? sum : v;
        best = (best >= current) ? best : current;
    }

    Result r = {1, best, NULL};
    return r;
}

static void run_test(int id, const int* input, size_t len, long long expected, int has_expected) {
    Result r = max_subarray_sum(input, len);
    printf("Test %d: input=[", id);
    for (size_t i = 0; i < len; i++) {
        if (i) printf(", ");
        printf("%d", input[i]);
    }
    printf("]\n");
    if (r.ok) {
        if (has_expected) {
            printf("  ok=true, max_sum=%lld, expected=%lld\n", r.value, expected);
        } else {
            printf("  ok=true, max_sum=%lld\n", r.value);
        }
    } else {
        printf("  ok=false, error=%s\n", r.error);
    }
}

int main(void) {
    int t1[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4}; /* 6 */
    int t2[] = {1, 2, 3, 4}; /* 10 */
    int t3[] = {-1, -2, -3}; /* -1 */
    int t4[] = {5, -9, 6, -2, 3}; /* 7 */
    /* t5: NULL with length 0 to simulate empty/error */
    const int* t5 = NULL;
    size_t t5_len = 0;

    run_test(1, t1, sizeof(t1)/sizeof(t1[0]), 6LL, 1);
    run_test(2, t2, sizeof(t2)/sizeof(t2[0]), 10LL, 1);
    run_test(3, t3, sizeof(t3)/sizeof(t3[0]), -1LL, 1);
    run_test(4, t4, sizeof(t4)/sizeof(t4[0]), 7LL, 1);
    run_test(5, t5, t5_len, 0LL, 0);

    return 0;
}