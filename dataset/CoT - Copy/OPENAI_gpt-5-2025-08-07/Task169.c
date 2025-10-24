#include <stdio.h>
#include <stdlib.h>

/* Binary search to find first index >= target; returns index in [0, n] */
static int lower_bound_int(const int* arr, int n, int target) {
    int l = 0, r = n;
    while (l < r) {
        int mid = l + ((r - l) >> 1);
        if (arr[mid] < target) l = mid + 1;
        else r = mid;
    }
    return l;
}

/* Compare function for qsort */
static int cmp_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

/* Counts of smaller elements to the right. Returns a heap-allocated array of size n. */
int* countSmaller(const int* nums, int n) {
    if (n <= 0) {
        int* empty = (int*)calloc(1, sizeof(int));
        return empty;
    }
    int* res = (int*)calloc((size_t)n, sizeof(int));
    if (!res) exit(1);

    // Coordinate compression
    int* tmp = (int*)malloc((size_t)n * sizeof(int));
    if (!tmp) exit(1);
    for (int i = 0; i < n; ++i) tmp[i] = nums[i];
    qsort(tmp, (size_t)n, sizeof(int), cmp_int);
    int* uniq = (int*)malloc((size_t)n * sizeof(int));
    if (!uniq) exit(1);
    int m = 0;
    for (int i = 0; i < n; ++i) {
        if (i == 0 || tmp[i] != tmp[i - 1]) {
            uniq[m++] = tmp[i];
        }
    }
    free(tmp);

    // Fenwick Tree
    int bitSize = m + 3;
    int* bit = (int*)calloc((size_t)bitSize, sizeof(int));
    if (!bit) exit(1);

    // Update BIT at index idx by delta (1-based)
    auto void update(int idx, int delta) {
        while (idx < bitSize) {
            bit[idx] += delta;
            idx += idx & -idx;
        }
    }
    // Query prefix sum up to idx (1-based)
    auto int query(int idx) {
        int s = 0;
        while (idx > 0) {
            s += bit[idx];
            idx -= idx & -idx;
        }
        return s;
    }

    for (int i = n - 1; i >= 0; --i) {
        int x = nums[i];
        int pos0 = lower_bound_int(uniq, m, x); // 0-based
        int idx1 = pos0 + 1; // 1-based
        res[i] = query(idx1 - 1);
        update(idx1, 1);
    }

    free(uniq);
    free(bit);
    return res;
}

/* Helper to print array */
static void print_array(const int* a, int n) {
    putchar('[');
    for (int i = 0; i < n; ++i) {
        if (i) putchar(',');
        printf("%d", a[i]);
    }
    puts("]");
}

int main(void) {
    // 5 test cases
    int t1[] = {5,2,6,1};        int n1 = 4;
    int t2[] = {-1};             int n2 = 1;
    int t3[] = {-1,-1};          int n3 = 2;
    int t4[] = {1,2,3,4};        int n4 = 4;
    int t5[] = {4,3,2,1};        int n5 = 4;

    int* r1 = countSmaller(t1, n1); print_array(r1, n1); free(r1);
    int* r2 = countSmaller(t2, n2); print_array(r2, n2); free(r2);
    int* r3 = countSmaller(t3, n3); print_array(r3, n3); free(r3);
    int* r4 = countSmaller(t4, n4); print_array(r4, n4); free(r4);
    int* r5 = countSmaller(t5, n5); print_array(r5, n5); free(r5);

    return 0;
}