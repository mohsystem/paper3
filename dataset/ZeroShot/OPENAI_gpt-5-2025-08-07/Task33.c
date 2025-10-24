#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

static void swap_ll(long long* a, long long* b) {
    long long t = *a; *a = *b; *b = t;
}

static void heapify_up(long long* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap[parent] <= heap[idx]) break;
        swap_ll(&heap[parent], &heap[idx]);
        idx = parent;
    }
}

static void heapify_down(long long* heap, int size, int idx) {
    while (1) {
        int left = 2 * idx + 1, right = 2 * idx + 2, smallest = idx;
        if (left < size && heap[left] < heap[smallest]) smallest = left;
        if (right < size && heap[right] < heap[smallest]) smallest = right;
        if (smallest == idx) break;
        swap_ll(&heap[idx], &heap[smallest]);
        idx = smallest;
    }
}

static void heap_push(long long* heap, int* size, long long val) {
    heap[*size] = val;
    (*size)++;
    heapify_up(heap, *size - 1);
}

static long long heap_pop_min(long long* heap, int* size) {
    if (*size <= 0) return 0;
    long long minv = heap[0];
    heap[0] = heap[*size - 1];
    (*size)--;
    heapify_down(heap, *size, 0);
    return minv;
}

// Calculates total checkout time given customer times and number of tills
int queueTime(const int* customers, int len, int n) {
    if (customers == NULL || len <= 0 || n <= 0) return 0;
    int tills = n < len ? n : len;
    long long* heap = (long long*)calloc((size_t)tills, sizeof(long long));
    if (!heap) return 0;
    int hsize = 0;

    for (int i = 0; i < tills; ++i) {
        long long t = customers[i] < 0 ? 0 : customers[i];
        heap_push(heap, &hsize, t);
    }
    for (int i = tills; i < len; ++i) {
        long long t = heap_pop_min(heap, &hsize);
        long long c = customers[i] < 0 ? 0 : customers[i];
        heap_push(heap, &hsize, t + c);
    }
    long long maxv = 0;
    for (int i = 0; i < hsize; ++i) if (heap[i] > maxv) maxv = heap[i];
    free(heap);
    if (maxv > INT_MAX) return INT_MAX;
    return (int)maxv;
}

int main(void) {
    // 5 test cases
    int a1[] = {5,3,4};
    printf("%d\n", queueTime(a1, 3, 1)); // 12

    int a2[] = {10,2,3,3};
    printf("%d\n", queueTime(a2, 4, 2)); // 10

    int a3[] = {2,3,10};
    printf("%d\n", queueTime(a3, 3, 2)); // 12

    int a4[] = {};
    printf("%d\n", queueTime(a4, 0, 1)); // 0

    int a5[] = {1,2,3,4,5};
    printf("%d\n", queueTime(a5, 5, 100)); // 5

    return 0;
}