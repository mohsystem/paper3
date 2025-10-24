#include <stdio.h>
#include <stddef.h>

int longestRun(const long long* arr, size_t n) {
    if (arr == NULL || n == 0) {
        return 0;
    }
    int maxLen = 1;
    int curLen = 1;
    int dir = 0; // 1 inc, -1 dec, 0 none
    for (size_t i = 1; i < n; ++i) {
        long long diff = arr[i] - arr[i - 1];
        if (diff == 1) {
            if (dir == 1 || dir == 0) {
                curLen += 1;
            } else {
                curLen = 2;
            }
            dir = 1;
        } else if (diff == -1) {
            if (dir == -1 || dir == 0) {
                curLen += 1;
            } else {
                curLen = 2;
            }
            dir = -1;
        } else {
            if (curLen > maxLen) {
                maxLen = curLen;
            }
            curLen = 1;
            dir = 0;
        }
        if (curLen > maxLen) {
            maxLen = curLen;
        }
    }
    if (curLen > maxLen) {
        maxLen = curLen;
    }
    return maxLen;
}

static void printArray(const long long* arr, size_t n) {
    putchar('[');
    for (size_t i = 0; i < n; ++i) {
        printf("%lld", arr[i]);
        if (i + 1 < n) {
            printf(", ");
        }
    }
    putchar(']');
}

int main(void) {
    long long t1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    long long t2[] = {1, 2, 3, 10, 11, 15};
    long long t3[] = {5, 4, 2, 1};
    long long t4[] = {3, 5, 7, 10, 15};
    long long t5[] = {1};

    const long long* tests[] = {t1, t2, t3, t4, t5};
    size_t sizes[] = {
        sizeof(t1) / sizeof(t1[0]),
        sizeof(t2) / sizeof(t2[0]),
        sizeof(t3) / sizeof(t3[0]),
        sizeof(t4) / sizeof(t4[0]),
        sizeof(t5) / sizeof(t5[0])
    };

    for (size_t i = 0; i < 5; ++i) {
        printf("Test %zu Input: ", i + 1);
        printArray(tests[i], sizes[i]);
        int result = longestRun(tests[i], sizes[i]);
        printf(" -> Longest Run: %d\n", result);
    }
    return 0;
}