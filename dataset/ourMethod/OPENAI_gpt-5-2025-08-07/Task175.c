#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MountainArray structure and operations */
typedef struct {
    int *data;
    int n;
    int get_count;
} MountainArray;

static MountainArray make_mountain_array(const int *arr, int n) {
    MountainArray ma;
    ma.n = (n < 0) ? 0 : n;
    ma.get_count = 0;
    if (ma.n > 0) {
        ma.data = (int *)malloc((size_t)ma.n * sizeof(int));
        if (ma.data == NULL) {
            ma.n = 0;
        } else {
            for (int i = 0; i < ma.n; ++i) {
                ma.data[i] = arr[i];
            }
        }
    } else {
        ma.data = NULL;
    }
    return ma;
}

static void destroy_mountain_array(MountainArray *ma) {
    if (ma && ma->data) {
        free(ma->data);
        ma->data = NULL;
        ma->n = 0;
        ma->get_count = 0;
    }
}

static int ma_length(const MountainArray *ma) {
    if (ma == NULL) return 0;
    return ma->n;
}

static int ma_get(MountainArray *ma, int index) {
    if (ma == NULL || ma->data == NULL || index < 0 || index >= ma->n) {
        fprintf(stderr, "Invalid get index: %d\n", index);
        return 0;
    }
    ma->get_count += 1;
    return ma->data[index];
}

static int ma_get_call_count(const MountainArray *ma) {
    if (ma == NULL) return 0;
    return ma->get_count;
}

/* Algorithm implementation */
static int find_peak(MountainArray *ma) {
    int l = 0, r = ma_length(ma) - 1;
    while (l < r) {
        int m = l + (r - l) / 2;
        int a = ma_get(ma, m);
        int b = ma_get(ma, m + 1);
        if (a < b) {
            l = m + 1;
        } else {
            r = m;
        }
    }
    return l;
}

static int bin_asc(MountainArray *ma, int l, int r, int target) {
    while (l <= r) {
        int m = l + (r - l) / 2;
        int v = ma_get(ma, m);
        if (v == target) return m;
        if (v < target) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

static int bin_desc(MountainArray *ma, int l, int r, int target) {
    while (l <= r) {
        int m = l + (r - l) / 2;
        int v = ma_get(ma, m);
        if (v == target) return m;
        if (v < target) r = m - 1;
        else l = m + 1;
    }
    return -1;
}

int find_in_mountain_array(int target, MountainArray *ma) {
    if (ma == NULL) return -1;
    int n = ma_length(ma);
    if (n < 3) return -1;
    int peak = find_peak(ma);
    int left = bin_asc(ma, 0, peak, target);
    if (left != -1) return left;
    return bin_desc(ma, peak + 1, n - 1, target);
}

/* Test harness */
static void run_test(const int *array, int n, int target, int expected) {
    MountainArray ma = make_mountain_array(array, n);
    int result = find_in_mountain_array(target, &ma);
    printf("Array: [");
    for (int i = 0; i < n; ++i) {
        printf("%d%s", array[i], (i + 1 < n) ? "," : "");
    }
    printf("], target=%d -> result=%d, expected=%d, getCalls=%d\n",
           target, result, expected, ma_get_call_count(&ma));
    destroy_mountain_array(&ma);
}

int main(void) {
    int a1[] = {1, 2, 3, 4, 5, 3, 1};
    int a2[] = {0, 1, 2, 4, 2, 1};
    int a3[] = {1, 3, 5, 7, 9, 6, 4, 2};
    int a4[] = {2, 5, 9, 12, 10, 7, 2};
    int a5[] = {0, 2, 5, 10, 9, 8, 1};

    run_test(a1, (int)(sizeof(a1)/sizeof(a1[0])), 3, 2);   // Example 1
    run_test(a2, (int)(sizeof(a2)/sizeof(a2[0])), 3, -1);  // Example 2
    run_test(a3, (int)(sizeof(a3)/sizeof(a3[0])), 1, 0);   // Target at start
    run_test(a4, (int)(sizeof(a4)/sizeof(a4[0])), 7, 5);   // Target on right
    run_test(a5, (int)(sizeof(a5)/sizeof(a5[0])), 10, 3);  // Target at peak

    return 0;
}