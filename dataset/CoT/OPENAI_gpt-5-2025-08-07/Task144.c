/* Chain-of-Through process:
 * 1) Problem understanding: Merge two sorted integer arrays into a single sorted array.
 * 2) Security requirements: Safely handle NULL pointers; check for size overflows; safe memory allocation and deallocation.
 * 3) Secure coding generation: Use size_t; validate sum for overflow before allocation; no user input.
 * 4) Code review: Loops, indices, and overflow protections verified; stable merge logic.
 * 5) Secure code output: Robust, leak-free, single-file program.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

int* merge_sorted_arrays(const int* a, size_t n, const int* b, size_t m, size_t* out_size) {
    if (out_size == NULL) {
        return NULL; // Cannot return size to caller safely
    }

    // Treat NULL arrays with non-zero sizes as invalid input; with zero sizes as empty
    if ((a == NULL && n != 0) || (b == NULL && m != 0)) {
        *out_size = 0;
        return NULL;
    }
    if (a == NULL) n = 0;
    if (b == NULL) m = 0;

    // Check for overflow in n + m and in multiplication for allocation
    if (n > SIZE_MAX - m) {
        *out_size = 0;
        return NULL; // overflow in sum
    }
    size_t total = n + m;
    if (total > SIZE_MAX / sizeof(int)) {
        *out_size = 0;
        return NULL; // overflow in bytes calculation
    }

    int* result = (int*)calloc(total, sizeof(int));
    if (result == NULL) {
        *out_size = 0;
        return NULL; // allocation failed
    }

    size_t i = 0, j = 0, k = 0;
    // Merge using two-pointer technique
    while (i < n && j < m) {
        if (a[i] <= b[j]) {
            result[k++] = a[i++];
        } else {
            result[k++] = b[j++];
        }
    }
    // Append remaining elements
    while (i < n) result[k++] = a[i++];
    while (j < m) result[k++] = b[j++];

    *out_size = total;
    return result;
}

static void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // 5 test cases
    int a1[] = {1, 3, 5};
    int b1[] = {2, 4, 6};

    int a2[] = {0, 2, 2, 2};
    int b2[] = {1, 3, 5, 7};

    int* a3 = NULL; size_t n3 = 0;
    int* b3 = NULL; size_t m3 = 0;

    int a4[] = {-5, -3, 0, 4};
    int b4[] = {-6, -4, -1, 2};

    int a5[] = {1,2,3,4,5,6,7,8,9,10};
    int* b5 = NULL; size_t m5 = 0;

    struct {
        const int* a; size_t n;
        const int* b; size_t m;
    } tests[5] = {
        {a1, sizeof(a1)/sizeof(a1[0]), b1, sizeof(b1)/sizeof(b1[0])},
        {a2, sizeof(a2)/sizeof(a2[0]), b2, sizeof(b2)/sizeof(b2[0])},
        {a3, n3, b3, m3},
        {a4, sizeof(a4)/sizeof(a4[0]), b4, sizeof(b4)/sizeof(b4[0])},
        {a5, sizeof(a5)/sizeof(a5[0]), b5, m5}
    };

    for (size_t t = 0; t < 5; ++t) {
        size_t out_n = 0;
        int* merged = merge_sorted_arrays(tests[t].a, tests[t].n, tests[t].b, tests[t].m, &out_n);
        if (merged != NULL || out_n == 0) {
            print_array(merged ? merged : (int[]){}, merged ? out_n : 0);
            free(merged);
        } else {
            // In case of failure with non-zero intended size, print empty to keep output consistent
            print_array((int[]){}, 0);
        }
    }

    return 0;
}