#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_VAL (-10000)
#define MAX_VAL (10000)
#define MAX_N   (100000)

typedef struct {
    int* data;
    int size;
    int error; /* 0 = ok, 1 = invalid input, 2 = memory error */
} IntArray;

static int cmp_int(const void* a, const void* b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

static int lower_bound_int(const int* arr, int len, int target) {
    int l = 0, r = len;
    while (l < r) {
        int mid = l + ((r - l) >> 1);
        if (arr[mid] < target) l = mid + 1;
        else r = mid;
    }
    return l;
}

static void bit_add(int* bit, int bit_len, int idx, int delta) {
    while (idx < bit_len) {
        bit[idx] += delta;
        idx += idx & -idx;
    }
}

static int bit_sum(const int* bit, int idx) {
    int s = 0;
    while (idx > 0) {
        s += bit[idx];
        idx -= idx & -idx;
    }
    return s;
}

/* Counts the number of smaller elements to the right of each element in nums.
   Returns IntArray with data (caller must free data when error == 0), size, and error code. */
IntArray count_smaller(const int* nums, int n) {
    IntArray out;
    out.data = NULL;
    out.size = 0;
    out.error = 0;

    if (nums == NULL || n < 0 || n > MAX_N) {
        out.error = 1;
        return out;
    }
    for (int i = 0; i < n; ++i) {
        if (nums[i] < MIN_VAL || nums[i] > MAX_VAL) {
            out.error = 1;
            return out;
        }
    }
    if (n == 0) {
        out.data = NULL;
        out.size = 0;
        out.error = 0;
        return out;
    }

    int* vals = (int*)malloc((size_t)n * sizeof(int));
    if (!vals) { out.error = 2; return out; }
    memcpy(vals, nums, (size_t)n * sizeof(int));
    qsort(vals, (size_t)n, sizeof(int), cmp_int);

    /* unique */
    int m = 0;
    for (int i = 0; i < n; ++i) {
        if (m == 0 || vals[m - 1] != vals[i]) {
            vals[m++] = vals[i];
        }
    }

    int bit_len = m + 2;
    int* bit = (int*)calloc((size_t)bit_len, sizeof(int));
    if (!bit) { free(vals); out.error = 2; return out; }

    int* res = (int*)malloc((size_t)n * sizeof(int));
    if (!res) { free(vals); free(bit); out.error = 2; return out; }

    for (int i = n - 1; i >= 0; --i) {
        int idx0 = lower_bound_int(vals, m, nums[i]);
        int idx = idx0 + 1; /* 1-based */
        res[i] = bit_sum(bit, idx - 1);
        bit_add(bit, bit_len, idx, 1);
    }

    free(vals);
    free(bit);

    out.data = res;
    out.size = n;
    out.error = 0;
    return out;
}

static void print_array(const int* arr, int n) {
    putchar('[');
    for (int i = 0; i < n; ++i) {
        if (i) putchar(',');
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int t1[] = {5, 2, 6, 1};
    int t2[] = {-1};
    int t3[] = {-1, -1};
    int t4[] = {1, 2, 3};
    int t5[] = {3, 2, 1};

    int* tests[] = {t1, t2, t3, t4, t5};
    int sizes[] = {4, 1, 2, 3, 3};

    for (int i = 0; i < 5; ++i) {
        IntArray ans = count_smaller(tests[i], sizes[i]);
        if (ans.error != 0) {
            printf("Error: %d\n", ans.error);
        } else {
            print_array(ans.data, ans.size);
            free(ans.data);
        }
    }
    return 0;
}