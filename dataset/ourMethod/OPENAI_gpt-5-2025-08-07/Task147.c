#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int ok;          /* 1 on success, 0 on failure */
    int value;       /* kth largest value if ok == 1 */
    char error[96];  /* error message if ok == 0 */
} KthResult;

static void heap_swap(int* a, int* b) {
    int t = *a; *a = *b; *b = t;
}

static void heap_sift_up(int* heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap[parent] <= heap[idx]) break;
        heap_swap(&heap[parent], &heap[idx]);
        idx = parent;
    }
}

static void heap_sift_down(int* heap, int size, int idx) {
    while (1) {
        int left = 2 * idx + 1;
        if (left >= size) break;
        int right = left + 1;
        int smallest = left;
        if (right < size && heap[right] < heap[left]) smallest = right;
        if (heap[idx] <= heap[smallest]) break;
        heap_swap(&heap[idx], &heap[smallest]);
        idx = smallest;
    }
}

KthResult kth_largest(const int* arr, size_t n, int k) {
    KthResult res;
    res.ok = 0;
    res.value = 0;
    res.error[0] = '\0';

    if (arr == NULL) {
        snprintf(res.error, sizeof(res.error), "%s", "Invalid input: array is NULL.");
        return res;
    }
    if (n == 0) {
        snprintf(res.error, sizeof(res.error), "%s", "Invalid input: array is empty.");
        return res;
    }
    if (k < 1 || (size_t)k > n) {
        snprintf(res.error, sizeof(res.error), "%s", "Invalid input: k out of range.");
        return res;
    }

    int* heap = (int*)malloc((size_t)k * sizeof(int));
    if (!heap) {
        snprintf(res.error, sizeof(res.error), "%s", "Memory allocation failure.");
        return res;
    }

    int size = 0;
    for (size_t i = 0; i < n; ++i) {
        int v = arr[i];
        if (size < k) {
            heap[size] = v;
            heap_sift_up(heap, size);
            size++;
        } else if (v > heap[0]) {
            heap[0] = v;
            heap_sift_down(heap, size, 0);
        }
    }

    if (size != k) {
        free(heap);
        snprintf(res.error, sizeof(res.error), "%s", "Unexpected error: heap size mismatch.");
        return res;
    }

    res.ok = 1;
    res.value = heap[0];
    free(heap);
    return res;
}

static void run_test(const int* arr, size_t n, int k) {
    KthResult r = kth_largest(arr, n, k);
    if (r.ok) {
        printf("k=%d, result=%d\n", k, r.value);
    } else {
        printf("k=%d, error=%s\n", k, r.error);
    }
}

int main(void) {
    int a1[] = {3, 2, 1, 5, 6, 4};
    int a2[] = {7, 10, 4, 3, 20, 15};
    int a3[] = {-1, -2, -3, -4};
    int a4[] = {5, 5, 5, 5};
    int a5[] = {1, 23, 12, 9, 30, 2, 50};
    int a6[] = {1};

    run_test(a1, sizeof(a1)/sizeof(a1[0]), 2);  /* expected 5 */
    run_test(a2, sizeof(a2)/sizeof(a2[0]), 3);  /* expected 10 */
    run_test(a3, sizeof(a3)/sizeof(a3[0]), 1);  /* expected -1 */
    run_test(a4, sizeof(a4)/sizeof(a4[0]), 2);  /* expected 5 */
    run_test(a5, sizeof(a5)/sizeof(a5[0]), 4);  /* expected 12 */
    /* Example invalid test (k out of range) */
    run_test(a6, sizeof(a6)/sizeof(a6[0]), 2);  /* error */

    return 0;
}