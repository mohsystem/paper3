#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Comparison function for qsort
static int compare_integers(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

/**
 * Sorts the positive numbers in an array in ascending order,
 * while keeping the negative numbers in their original positions.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return A new dynamically allocated array with positive numbers sorted.
 *         The caller is responsible for freeing this memory.
 *         Returns NULL on allocation failure or if arr is NULL for size > 0.
 */
int* posNegSort(const int* arr, size_t size) {
    if (size == 0) {
        return (int*)malloc(0);
    }
    if (arr == NULL) {
        return NULL;
    }

    size_t pos_count = 0;
    for (size_t i = 0; i < size; i++) {
        if (arr[i] > 0) {
            pos_count++;
        }
    }

    if (pos_count == 0) {
        int* result_copy = (int*)malloc(size * sizeof(int));
        if (result_copy == NULL) return NULL;
        memcpy(result_copy, arr, size * sizeof(int));
        return result_copy;
    }

    int* positives = (int*)malloc(pos_count * sizeof(int));
    if (positives == NULL) {
        return NULL;
    }

    size_t current_pos_idx = 0;
    for (size_t i = 0; i < size; i++) {
        if (arr[i] > 0) {
            positives[current_pos_idx++] = arr[i];
        }
    }

    qsort(positives, pos_count, sizeof(int), compare_integers);

    int* result = (int*)malloc(size * sizeof(int));
    if (result == NULL) {
        free(positives);
        return NULL;
    }

    current_pos_idx = 0;
    for (size_t i = 0; i < size; i++) {
        if (arr[i] > 0) {
            result[i] = positives[current_pos_idx++];
        } else {
            result[i] = arr[i];
        }
    }

    free(positives);
    return result;
}

void print_array(const int* arr, size_t size) {
    printf("[");
    for (size_t i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

void run_test_case(const int* test_arr, size_t size) {
    int* result = posNegSort(test_arr, size);
    printf("Original: "); print_array(test_arr, size); printf("\n");
    if (result != NULL) {
        printf("Sorted:   "); print_array(result, size); printf("\n\n");
        free(result);
    } else {
        printf("Sorted:   (null)\n\n");
    }
}

int main() {
    int test1[] = {6, 3, -2, 5, -8, 2, -2};
    run_test_case(test1, sizeof(test1) / sizeof(test1[0]));

    int test2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    run_test_case(test2, sizeof(test2) / sizeof(test2[0]));

    int test3[] = {-5, -5, -5, -5, 7, -5};
    run_test_case(test3, sizeof(test3) / sizeof(test3[0]));

    int* test4 = NULL;
    run_test_case(test4, 0);

    int test5[] = {1, -1, 2, -2, 3, -3};
    run_test_case(test5, sizeof(test5) / sizeof(test5[0]));

    return 0;
}