#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

static const size_t MAX_LEN = 1000000;

static void swap_int(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
}

static void sift_down(int* arr, size_t start, size_t heap_size) {
    size_t root = start;
    while (true) {
        size_t left = 2 * root + 1;
        if (left >= heap_size) break;
        size_t right = left + 1;
        size_t largest = root;
        if (arr[left] > arr[largest]) largest = left;
        if (right < heap_size && arr[right] > arr[largest]) largest = right;
        if (largest == root) break;
        swap_int(&arr[root], &arr[largest]);
        root = largest;
    }
}

int* heap_sort(const int* input, size_t n, size_t* out_n) {
    if (out_n == NULL) {
        return NULL;
    }
    *out_n = 0;
    if ((n > 0 && input == NULL) || n > MAX_LEN) {
        return NULL;
    }

    if (n == 0) {
        *out_n = 0;
        return NULL;
    }

    int* arr = (int*)malloc(n * sizeof(int));
    if (arr == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < n; ++i) {
        arr[i] = input[i];
    }

    // Build max heap
    if (n > 1) {
        for (size_t i = n / 2; i-- > 0;) {
            sift_down(arr, i, n);
        }
    }

    // Extract elements
    for (size_t end = n; end > 1; --end) {
        swap_int(&arr[0], &arr[end - 1]);
        sift_down(arr, 0, end - 1);
    }

    *out_n = n;
    return arr;
}

static void print_array(const int* arr, size_t n) {
    putchar('[');
    for (size_t i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    putchar(']');
    putchar('\n');
}

int main(void) {
    int t1[] = {3, 1, 4, 1, 5, 9, 2, 6, 5};
    int t2[] = {};
    int t3[] = {42};
    int t4[] = {1, 2, 3, 4, 5};
    int t5[] = {0, -1, -3, 2, -1, 2, 0};

    int* sorted = NULL;
    size_t n_sorted = 0;

    sorted = heap_sort(t1, sizeof(t1)/sizeof(t1[0]), &n_sorted);
    print_array(sorted, n_sorted);
    free(sorted);

    sorted = heap_sort(t2, sizeof(t2)/sizeof(t2[0]), &n_sorted);
    print_array(sorted, n_sorted);
    free(sorted);

    sorted = heap_sort(t3, sizeof(t3)/sizeof(t3[0]), &n_sorted);
    print_array(sorted, n_sorted);
    free(sorted);

    sorted = heap_sort(t4, sizeof(t4)/sizeof(t4[0]), &n_sorted);
    print_array(sorted, n_sorted);
    free(sorted);

    sorted = heap_sort(t5, sizeof(t5)/sizeof(t5[0]), &n_sorted);
    print_array(sorted, n_sorted);
    free(sorted);

    return 0;
}