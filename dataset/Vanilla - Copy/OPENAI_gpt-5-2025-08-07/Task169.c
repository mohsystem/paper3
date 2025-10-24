#include <stdio.h>
#include <stdlib.h>

static int cmp_int(const void* a, const void* b) {
    int x = *(const int*)a, y = *(const int*)b;
    return (x > y) - (x < y);
}

static int lower_bound_int(int* arr, int n, int target) {
    int l = 0, r = n;
    while (l < r) {
        int m = l + (r - l) / 2;
        if (arr[m] < target) l = m + 1;
        else r = m;
    }
    return l;
}

static void bit_update(int* bit, int size, int i, int delta) {
    for (; i <= size; i += i & -i) bit[i] += delta;
}

static int bit_query(int* bit, int i) {
    int s = 0;
    for (; i > 0; i -= i & -i) s += bit[i];
    return s;
}

int* countSmaller(int* nums, int n, int* returnSize) {
    *returnSize = n;
    int* res = (int*)calloc(n, sizeof(int));
    if (n == 0) return res;

    int* tmp = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; ++i) tmp[i] = nums[i];
    qsort(tmp, n, sizeof(int), cmp_int);

    int* uniq = (int*)malloc(n * sizeof(int));
    int m = 0;
    for (int i = 0; i < n; ++i) {
        if (m == 0 || uniq[m - 1] != tmp[i]) uniq[m++] = tmp[i];
    }
    free(tmp);

    int bitSize = m + 2;
    int* bit = (int*)calloc(bitSize + 1, sizeof(int));

    for (int i = n - 1; i >= 0; --i) {
        int id = lower_bound_int(uniq, m, nums[i]) + 1;
        res[i] = bit_query(bit, id - 1);
        bit_update(bit, bitSize, id, 1);
    }

    free(uniq);
    free(bit);
    return res;
}

static void printArray(int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    int t1[] = {5,2,6,1};
    int t2[] = {-1};
    int t3[] = {-1,-1};
    int t4[] = {3,2,2,6,1};
    int t5[] = {1,0,2,2,5};

    int* tests[] = {t1, t2, t3, t4, t5};
    int sizes[] = {4, 1, 2, 5, 5};

    for (int i = 0; i < 5; ++i) {
        int rs = 0;
        int* res = countSmaller(tests[i], sizes[i], &rs);
        printArray(res, rs);
        free(res);
    }
    return 0;
}