#include <stdio.h>

int findEvenIndex(const int* arr, int n) {
    long long total = 0;
    for (int i = 0; i < n; ++i) total += arr[i];
    long long left = 0;
    for (int i = 0; i < n; ++i) {
        long long right = total - left - arr[i];
        if (left == right) return i;
        left += arr[i];
    }
    return -1;
}

int main(void) {
    int t1[] = {1, 2, 3, 4, 3, 2, 1};
    int t2[] = {1, 100, 50, -51, 1, 1};
    int t3[] = {20, 10, -80, 10, 10, 15, 35};
    int t4[] = {1, 2, 3, 4, 5, 6};
    int t5[] = {0, 0, 0, 0};

    int* tests[] = {t1, t2, t3, t4, t5};
    int sizes[] = {7, 6, 7, 6, 4};

    for (int i = 0; i < 5; ++i) {
        printf("[");
        for (int j = 0; j < sizes[i]; ++j) {
            printf("%d%s", tests[i][j], (j + 1 < sizes[i]) ? ", " : "");
        }
        printf("] -> %d\n", findEvenIndex(tests[i], sizes[i]));
    }
    return 0;
}