#include <stdio.h>
#include <stddef.h>

typedef struct {
    int first;
    int second;
} Pair;

Pair two_sum(const int* arr, size_t n, int target) {
    Pair res;
    res.first = -1;
    res.second = -1;
    if (arr == NULL || n < 2) {
        return res;
    }
    for (size_t i = 0; i + 1 < n; ++i) {
        int a = arr[i];
        for (size_t j = i + 1; j < n; ++j) {
            long long sum = (long long)a + (long long)arr[j];
            if (sum == (long long)target) {
                res.first = (int)i;
                res.second = (int)j;
                return res;
            }
        }
    }
    return res;
}

static void print_pair(Pair p) {
    printf("{%d, %d}\n", p.first, p.second);
}

int main(void) {
    int a1[] = {1, 2, 3};
    print_pair(two_sum(a1, sizeof(a1)/sizeof(a1[0]), 4)); // {0, 2}

    int a2[] = {3, 2, 4};
    print_pair(two_sum(a2, sizeof(a2)/sizeof(a2[0]), 6)); // {1, 2}

    int a3[] = {3, 3};
    print_pair(two_sum(a3, sizeof(a3)/sizeof(a3[0]), 6)); // {0, 1}

    int a4[] = {-1, -2, -3, -4};
    print_pair(two_sum(a4, sizeof(a4)/sizeof(a4[0]), -6)); // {1, 3} or valid

    int a5[] = {2, 7, 11, 15};
    print_pair(two_sum(a5, sizeof(a5)/sizeof(a5[0]), 9)); // {0, 1}

    return 0;
}