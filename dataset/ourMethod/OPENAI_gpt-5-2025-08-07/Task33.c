#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef long long i64;

static void swap_ll(i64* a, i64* b) {
    i64 t = *a; *a = *b; *b = t;
}

static void heapify_up(i64* heap, size_t idx) {
    while (idx > 0) {
        size_t parent = (idx - 1) / 2;
        if (heap[parent] <= heap[idx]) break;
        swap_ll(&heap[parent], &heap[idx]);
        idx = parent;
    }
}

static void heapify_down(i64* heap, size_t size, size_t idx) {
    while (1) {
        size_t left = 2 * idx + 1;
        size_t right = 2 * idx + 2;
        size_t smallest = idx;
        if (left < size && heap[left] < heap[smallest]) smallest = left;
        if (right < size && heap[right] < heap[smallest]) smallest = right;
        if (smallest == idx) break;
        swap_ll(&heap[idx], &heap[smallest]);
        idx = smallest;
    }
}

static int heap_push(i64* heap, size_t* size, size_t capacity, i64 value) {
    if (*size >= capacity) return 0;
    heap[*size] = value;
    (*size)++;
    heapify_up(heap, (*size) - 1);
    return 1;
}

static int heap_pop_min(i64* heap, size_t* size, i64* out_min) {
    if (*size == 0) return 0;
    *out_min = heap[0];
    heap[0] = heap[(*size) - 1];
    (*size)--;
    if (*size > 0) heapify_down(heap, *size, 0);
    return 1;
}

long long queueTimeC(const int* customers, size_t len, int n) {
    if (n <= 0) return -1;
    if (len > 0 && customers == NULL) return -1;

    for (size_t i = 0; i < len; ++i) {
        if (customers[i] < 0) return -1;
    }
    if (len == 0) return 0;

    size_t m = (size_t)n;
    if (m > len) m = len;

    i64* heap = (i64*)malloc(sizeof(i64) * m);
    if (heap == NULL) return -1;

    size_t hsize = 0;
    // Initialize tills
    for (size_t i = 0; i < m; ++i) {
        if (!heap_push(heap, &hsize, m, (i64)customers[i])) {
            free(heap);
            return -1;
        }
    }
    // Process remaining customers
    for (size_t i = m; i < len; ++i) {
        i64 tmin = 0;
        if (!heap_pop_min(heap, &hsize, &tmin)) {
            free(heap);
            return -1;
        }
        if ((i64)customers[i] > LLONG_MAX - tmin) {
            free(heap);
            return -1; // overflow
        }
        i64 tsum = tmin + (i64)customers[i];
        if (!heap_push(heap, &hsize, m, tsum)) {
            free(heap);
            return -1;
        }
    }

    // Compute max finish time from heap
    i64 total = 0;
    for (size_t i = 0; i < hsize; ++i) {
        if (heap[i] > total) total = heap[i];
    }

    free(heap);
    return total;
}

int main(void) {
    struct {
        const int* arr;
        size_t len;
        int n;
    } tests[5];

    static const int t1[] = {5,3,4};
    static const int t2[] = {10,2,3,3};
    static const int t3[] = {2,3,10};
    static const int t4[] = {};
    static const int t5[] = {1,2,3,4,5};

    tests[0].arr = t1; tests[0].len = sizeof(t1)/sizeof(t1[0]); tests[0].n = 1;
    tests[1].arr = t2; tests[1].len = sizeof(t2)/sizeof(t2[0]); tests[1].n = 2;
    tests[2].arr = t3; tests[2].len = sizeof(t3)/sizeof(t3[0]); tests[2].n = 2;
    tests[3].arr = t4; tests[3].len = sizeof(t4)/sizeof(t4[0]); tests[3].n = 1;
    tests[4].arr = t5; tests[4].len = sizeof(t5)/sizeof(t5[0]); tests[4].n = 100;

    for (int i = 0; i < 5; ++i) {
        long long res = queueTimeC(tests[i].arr, tests[i].len, tests[i].n);
        printf("Test %d: %lld\n", i+1, res);
    }

    return 0;
}