#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

int* removeSmallest(const int* arr, size_t n, size_t* out_n) {
    if (out_n) *out_n = 0;
    if (arr == NULL || n == 0) {
        return NULL;
    }
    size_t minIdx = 0;
    int minVal = arr[0];
    for (size_t i = 1; i < n; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = i;
        }
    }
    if (n - 1 == 0) {
        if (out_n) *out_n = 0;
        return NULL;
    }
    int* result = (int*)malloc(sizeof(int) * (n - 1));
    if (result == NULL) {
        if (out_n) *out_n = 0;
        return NULL;
    }
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i == minIdx) continue;
        result[j++] = arr[i];
    }
    if (out_n) *out_n = n - 1;
    return result;
}

void printArray(const int* a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", a[i]);
    }
    printf("]\n");
}

int main(void) {
    int t1[] = {1, 2, 3, 4, 5};
    int t2[] = {5, 3, 2, 1, 4};
    int t3[] = {2, 2, 1, 2, 1};
    int* t4 = NULL; size_t n4 = 0;
    int t5[] = {7};

    size_t out_n;
    int* res;

    res = removeSmallest(t1, sizeof(t1)/sizeof(t1[0]), &out_n);
    printArray(res, out_n);
    free(res);

    res = removeSmallest(t2, sizeof(t2)/sizeof(t2[0]), &out_n);
    printArray(res, out_n);
    free(res);

    res = removeSmallest(t3, sizeof(t3)/sizeof(t3[0]), &out_n);
    printArray(res, out_n);
    free(res);

    res = removeSmallest(t4, n4, &out_n);
    printArray(res, out_n);
    free(res);

    res = removeSmallest(t5, sizeof(t5)/sizeof(t5[0]), &out_n);
    printArray(res, out_n);
    free(res);

    return 0;
}