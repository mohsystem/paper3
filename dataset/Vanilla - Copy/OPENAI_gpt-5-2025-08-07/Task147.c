#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int cmp_ints(const void* a, const void* b) {
    int ai = *(const int*)a;
    int bi = *(const int*)b;
    if (ai < bi) return -1;
    if (ai > bi) return 1;
    return 0;
}

int kthLargest(const int* arr, int n, int k) {
    if (arr == NULL || k < 1 || k > n) {
        return INT_MIN; // indicates invalid input
    }
    int* copy = (int*)malloc(n * sizeof(int));
    if (!copy) return INT_MIN;
    for (int i = 0; i < n; ++i) copy[i] = arr[i];
    qsort(copy, n, sizeof(int), cmp_ints);
    int result = copy[n - k];
    free(copy);
    return result;
}

int main() {
    int a1[] = {3, 2, 1, 5, 6, 4};          int k1 = 2;
    int a2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6}; int k2 = 4;
    int a3[] = {7};                         int k3 = 1;
    int a4[] = {-1, -2, -3, -4};            int k4 = 2;
    int a5[] = {5, 5, 5, 5};                int k5 = 3;

    printf("%d\n", kthLargest(a1, sizeof(a1)/sizeof(a1[0]), k1));
    printf("%d\n", kthLargest(a2, sizeof(a2)/sizeof(a2[0]), k2));
    printf("%d\n", kthLargest(a3, sizeof(a3)/sizeof(a3[0]), k3));
    printf("%d\n", kthLargest(a4, sizeof(a4)/sizeof(a4[0]), k4));
    printf("%d\n", kthLargest(a5, sizeof(a5)/sizeof(a5[0]), k5));
    return 0;
}