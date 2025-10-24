#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

static bool is_non_decreasing(const int* arr, size_t n) {
    if (n == 0) return true;
    for (size_t i = 1; i < n; ++i) {
        if (arr[i - 1] > arr[i]) return false;
    }
    return true;
}

int* merge_sorted(const int* a, size_t n, const int* b, size_t m, size_t* out_len) {
    if (out_len == NULL) {
        return NULL;
    }
    *out_len = 0;

    if ((n > 0 && a == NULL) || (m > 0 && b == NULL)) {
        return NULL;
    }
    if (n > SIZE_MAX - m) {
        return NULL; // overflow
    }

    size_t total = n + m;
    int* result = NULL;
    if (total > 0) {
        result = (int*)malloc(total * sizeof(int));
        if (result == NULL) {
            return NULL;
        }
    } else {
        // both empty
        *out_len = 0;
        return result; // NULL for empty is acceptable
    }

    if (!is_non_decreasing(a, n) || !is_non_decreasing(b, m)) {
        free(result);
        return NULL;
    }

    size_t i = 0, j = 0, k = 0;
    while (i < n && j < m) {
        if (a[i] <= b[j]) {
            result[k++] = a[i++];
        } else {
            result[k++] = b[j++];
        }
    }
    while (i < n) result[k++] = a[i++];
    while (j < m) result[k++] = b[j++];

    *out_len = total;
    return result;
}

static void print_array(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) {
            printf(", ");
        }
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main(void) {
    // Test case 1: both empty
    {
        size_t out_n = 0;
        int* merged = merge_sorted(NULL, 0, NULL, 0, &out_n);
        if (merged == NULL && out_n == 0) {
            print_array(NULL, 0);
        } else {
            print_array(merged, out_n);
            free(merged);
        }
    }

    // Test case 2: one empty (b empty)
    {
        int a2[] = {1, 3, 5};
        size_t out_n = 0;
        int* merged = merge_sorted(a2, sizeof(a2)/sizeof(a2[0]), NULL, 0, &out_n);
        if (merged == NULL && out_n == 0) {
            // Should not happen for valid input
            print_array(NULL, 0);
        } else {
            print_array(merged, out_n);
            free(merged);
        }
    }

    // Test case 3: one empty (a empty)
    {
        int b3[] = {2, 4, 6};
        size_t out_n = 0;
        int* merged = merge_sorted(NULL, 0, b3, sizeof(b3)/sizeof(b3[0]), &out_n);
        if (merged == NULL && out_n == 0) {
            // Should not happen for valid input
            print_array(NULL, 0);
        } else {
            print_array(merged, out_n);
            free(merged);
        }
    }

    // Test case 4: duplicates
    {
        int a4[] = {1, 2, 2, 3};
        int b4[] = {2, 2, 4};
        size_t out_n = 0;
        int* merged = merge_sorted(a4, sizeof(a4)/sizeof(a4[0]), b4, sizeof(b4)/sizeof(b4[0]), &out_n);
        if (merged == NULL) {
            printf("Invalid input\n");
        } else {
            print_array(merged, out_n);
            free(merged);
        }
    }

    // Test case 5: negatives and positives
    {
        int a5[] = {-5, -2, 0};
        int b5[] = {-3, -1, 2};
        size_t out_n = 0;
        int* merged = merge_sorted(a5, sizeof(a5)/sizeof(a5[0]), b5, sizeof(b5)/sizeof(b5[0]), &out_n);
        if (merged == NULL) {
            printf("Invalid input\n");
        } else {
            print_array(merged, out_n);
            free(merged);
        }
    }

    return 0;
}