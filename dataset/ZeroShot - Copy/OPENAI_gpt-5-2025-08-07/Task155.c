#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int* data;
    size_t size;
} IntArray;

static void swap_int(int* a, int* b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static void sift_down(int* a, size_t heap_size, size_t i) {
    while (1) {
        size_t largest = i;
        size_t left = i * 2 + 1;
        size_t right = left + 1;

        if (left < heap_size && a[left] > a[largest]) largest = left;
        if (right < heap_size && a[right] > a[largest]) largest = right;
        if (largest == i) break;
        swap_int(&a[i], &a[largest]);
        i = largest;
    }
}

static IntArray heap_sort(const int* arr, size_t n) {
    IntArray out = {NULL, 0};
    if (arr == NULL || n == 0) {
        out.data = NULL;
        out.size = 0;
        return out;
    }

    int* a = (int*)malloc(n * sizeof(int));
    if (!a) {
        // Allocation failed; return empty safely
        out.data = NULL;
        out.size = 0;
        return out;
    }
    memcpy(a, arr, n * sizeof(int));

    if (n > 1) {
        for (size_t i = n / 2; i-- > 0; ) {
            sift_down(a, n, i);
        }
        for (size_t end = n - 1; end > 0; --end) {
            swap_int(&a[0], &a[end]);
            sift_down(a, end, 0);
        }
    }

    out.data = a;
    out.size = n;
    return out;
}

static void print_array(const int* a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", a[i]);
    }
    printf("]\n");
}

int main(void) {
    int t0[] = {};
    int t1[] = {1};
    int t2[] = {5, 3, 8, 4, 2, 7, 1, 10};
    int t3[] = {1, 2, 3, 4, 5};
    int t4[] = {-3, -1, -7, 0, 2, 2, -3};

    struct {
        int* data;
        size_t size;
    } tests[] = {
        {t0, 0},
        {t1, sizeof(t1)/sizeof(t1[0])},
        {t2, sizeof(t2)/sizeof(t2[0])},
        {t3, sizeof(t3)/sizeof(t3[0])},
        {t4, sizeof(t4)/sizeof(t4[0])}
    };

    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        IntArray res = heap_sort(tests[i].data, tests[i].size);
        print_array(res.data, res.size);
        free(res.data);
    }

    return 0;
}