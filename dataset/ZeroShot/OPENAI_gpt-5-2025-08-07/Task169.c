#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Fenwick Tree (Binary Indexed Tree) */
typedef struct {
    int n;
    int *bit;
} Fenwick;

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

static Fenwick fenwick_create(int n) {
    Fenwick f;
    f.n = n;
    f.bit = (int*)calloc((size_t)n + 1, sizeof(int));
    if (!f.bit) die("Memory allocation failed for Fenwick tree.");
    return f;
}

static void fenwick_update(Fenwick *f, int idx, int delta) {
    for (int i = idx; i <= f->n; i += i & -i) {
        f->bit[i] += delta;
    }
}

static int fenwick_query(const Fenwick *f, int idx) {
    int s = 0;
    for (int i = idx; i > 0; i -= i & -i) {
        s += f->bit[i];
    }
    return s;
}

static void fenwick_free(Fenwick *f) {
    if (f && f->bit) {
        free(f->bit);
        f->bit = NULL;
        f->n = 0;
    }
}

static int cmp_int(const void *a, const void *b) {
    int x = *(const int*)a;
    int y = *(const int*)b;
    return (x > y) - (x < y);
}

static size_t lower_bound_int(const int *arr, size_t n, int target) {
    size_t lo = 0, hi = n;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (arr[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

/* Returns a newly allocated array of counts, sets *out_len to n. Caller must free the returned array. */
int* countSmaller(const int* nums, size_t n, size_t* out_len) {
    if (!out_len) die("out_len pointer is NULL.");
    *out_len = n;
    if (nums == NULL || n == 0) {
        int* empty = (int*)calloc(1, sizeof(int));
        if (!empty) die("Memory allocation failed.");
        *out_len = 0;
        return empty;
    }

    int* sorted = (int*)malloc(n * sizeof(int));
    if (!sorted) die("Memory allocation failed.");
    memcpy(sorted, nums, n * sizeof(int));
    qsort(sorted, n, sizeof(int), cmp_int);

    // Deduplicate
    size_t m = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i == 0 || sorted[i] != sorted[i - 1]) {
            sorted[m++] = sorted[i];
        }
    }

    int* ans = (int*)malloc(n * sizeof(int));
    if (!ans) {
        free(sorted);
        die("Memory allocation failed.");
    }

    Fenwick ft = fenwick_create((int)m);

    for (size_t i = n; i-- > 0; ) {
        size_t r0 = lower_bound_int(sorted, m, nums[i]); // 0-based rank
        int r = (int)r0 + 1; // 1-based for Fenwick
        ans[i] = fenwick_query(&ft, r - 1);
        fenwick_update(&ft, r, 1);
    }

    fenwick_free(&ft);
    free(sorted);
    return ans;
}

static void print_array(const int* a, size_t n) {
    putchar('[');
    for (size_t i = 0; i < n; ++i) {
        if (i) putchar(',');
        printf("%d", a[i]);
    }
    putchar(']');
}

int main(void) {
    int t1[] = {5,2,6,1};
    int t2[] = {-1};
    int t3[] = {-1,-1};
    int t4[] = {3,2,2,6,1};
    int t5[] = {2,0,1};

    int* tests[] = { t1, t2, t3, t4, t5 };
    size_t lens[] = { 4, 1, 2, 5, 3 };

    for (size_t k = 0; k < 5; ++k) {
        size_t out_len = 0;
        int* res = countSmaller(tests[k], lens[k], &out_len);
        printf("Input: ");
        print_array(tests[k], lens[k]);
        printf("\nOutput: ");
        print_array(res, out_len);
        printf("\n---\n");
        free(res);
    }
    return 0;
}