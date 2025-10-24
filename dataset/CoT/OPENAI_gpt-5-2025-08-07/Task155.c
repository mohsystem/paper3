/* Chain-of-Through process:
1) Problem understanding: Implement heap sort in C that returns a new allocated sorted array.
2) Security requirements: Validate pointers, handle n safely, avoid out-of-bounds and memory issues.
3) Secure coding generation: Allocate exact memory, check allocation, iterative heapify.
4) Code review: Verified checks for NULL, n, and memory management.
5) Secure code output: Final code with tests and proper free().
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void swap_int(int* a, int* b) {
    if (a == NULL || b == NULL) return;
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heapify(int* a, size_t heapSize, size_t i) {
    if (a == NULL) return;
    while (1) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = 2 * i + 2;

        if (left < heapSize && a[left] > a[largest]) {
            largest = left;
        }
        if (right < heapSize && a[right] > a[largest]) {
            largest = right;
        }
        if (largest != i) {
            swap_int(&a[i], &a[largest]);
            i = largest;
        } else {
            break;
        }
    }
}

int* heap_sort(const int* arr, size_t n) {
    if (arr == NULL && n > 0) {
        return NULL;
    }
    if (n == 0) {
        // Return a valid pointer or NULL; returning NULL for empty is acceptable as length is known to caller.
        return NULL;
    }
    int* a = (int*)malloc(n * sizeof(int));
    if (a == NULL) {
        return NULL; // Allocation failed
    }
    memcpy(a, arr, n * sizeof(int));

    // Build max heap
    if (n >= 2) {
        size_t i = n / 2;
        do {
            i--;
            heapify(a, n, i);
        } while (i > 0);
    }

    // Heap sort
    for (size_t i = n; i-- > 1; ) {
        swap_int(&a[0], &a[i - 1]);
        heapify(a, i - 1, 0);
    }
    return a;
}

static void print_array(const int* arr, size_t n) {
    if (arr == NULL && n == 0) {
        printf("[]");
        return;
    }
    if (arr == NULL) {
        printf("null");
        return;
    }
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("%d", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]");
}

int main(void) {
    int test1[] = {4, 10, 3, 5, 1};
    int test2[] = {};
    int test3[] = {1};
    int test4[] = {5, -1, 3, -2, 0, 5, 5};
    int test5[] = {1000000000, -1000000000, INT_MAX, INT_MIN, 42};

    int* tests[] = { test1, test2, test3, test4, test5 };
    size_t lens[] = { 5, 0, 1, 7, 5 };

    for (size_t i = 0; i < 5; ++i) {
        int* sorted = heap_sort(tests[i], lens[i]);
        printf("Test %zu input:  ", i + 1);
        print_array(tests[i], lens[i]);
        printf("\nTest %zu sorted: ", i + 1);
        print_array(sorted, lens[i]);
        printf("\n\n");
        free(sorted);
    }
    return 0;
}