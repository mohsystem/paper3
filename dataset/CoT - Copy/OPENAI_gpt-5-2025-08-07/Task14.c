#include <stdio.h>
#include <stddef.h>

// Step 1: Problem understanding
// Find the lowest index N where sum of left elements equals sum of right elements.

// Step 2-3: Security requirements and Secure coding generation
// - Use long long for sums to reduce overflow risk.
// - Validate pointers and sizes defensively.

int find_even_index(const int* arr, size_t n) {
    if (arr == NULL || n == 0) {
        return -1;
    }
    long long total = 0;
    for (size_t i = 0; i < n; ++i) {
        total += arr[i];
    }
    long long left = 0;
    for (size_t i = 0; i < n; ++i) {
        long long right = total - left - arr[i];
        if (left == right) {
            if (i > 2147483647u) { // guard cast to int
                return -1;
            }
            return (int)i;
        }
        left += arr[i];
    }
    return -1;
}

// Step 4: Code review
// - Loops are bounded by n.
// - Casting to int is checked for overflow.

// Step 5: Secure code output
int main(void) {
    int t1[] = {1,2,3,4,3,2,1};              // expected 3
    int t2[] = {1,100,50,-51,1,1};           // expected 1
    int t3[] = {20,10,-80,10,10,15,35};      // expected 0
    int t4[] = {1,2,3,4,5,6};                // expected -1
    int t5[] = {2,0,2};                      // expected 1

    const int* tests[] = {t1, t2, t3, t4, t5};
    size_t sizes[] = {sizeof(t1)/sizeof(t1[0]),
                      sizeof(t2)/sizeof(t2[0]),
                      sizeof(t3)/sizeof(t3[0]),
                      sizeof(t4)/sizeof(t4[0]),
                      sizeof(t5)/sizeof(t5[0])};

    for (size_t i = 0; i < 5; ++i) {
        int res = find_even_index(tests[i], sizes[i]);
        printf("Test %zu [", i + 1);
        for (size_t j = 0; j < sizes[i]; ++j) {
            printf("%d", tests[i][j]);
            if (j + 1 < sizes[i]) printf(",");
        }
        printf("] => %d\n", res);
    }
    return 0;
}