#include <stdio.h>
#include <stdlib.h>

/**
 * Given an array of integers, removes the smallest value.
 * This function does not mutate the original array.
 * Caller is responsible for freeing the memory of the returned array.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the input array.
 * @param new_size A pointer to an integer where the size of the new array will be stored.
 * @return A new, dynamically allocated array with the smallest value removed.
 *         Returns NULL for an empty resulting array.
 */
int* removeSmallest(const int* arr, int size, int* new_size) {
    // If input array is empty or has one element, the result is an empty array.
    if (size <= 1) {
        *new_size = 0;
        return NULL;
    }

    // Find the index of the first occurrence of the minimum value.
    int min_index = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] < arr[min_index]) {
            min_index = i;
        }
    }

    // Allocate memory for the new array (size - 1).
    *new_size = size - 1;
    int* result = (int*)malloc(*new_size * sizeof(int));
    if (result == NULL) {
        // Handle malloc failure gracefully.
        *new_size = 0;
        return NULL;
    }

    // Copy elements, skipping the one at min_index.
    int result_index = 0;
    for (int i = 0; i < size; i++) {
        if (i != min_index) {
            result[result_index++] = arr[i];
        }
    }

    return result;
}

// Helper function to print an array
void printArray(const int* arr, int size) {
    printf("[");
    if (size > 0 && arr != NULL) {
        for (int i = 0; i < size; i++) {
            printf("%d", arr[i]);
            if (i < size - 1) {
                printf(", ");
            }
        }
    }
    printf("]\n");
}

// Helper to run a test case and manage memory
void run_test_case(const int* test_arr, int size) {
    int new_size;
    int* result = removeSmallest(test_arr, size, &new_size);
    printArray(result, new_size);
    free(result); // Free the memory allocated by removeSmallest
}

int main() {
    int test1[] = {1, 2, 3, 4, 5};
    run_test_case(test1, 5);

    int test2[] = {5, 3, 2, 1, 4};
    run_test_case(test2, 5);

    int test3[] = {2, 2, 1, 2, 1};
    run_test_case(test3, 5);

    int test4[] = {1};
    run_test_case(test4, 1);
    
    int* test5 = NULL;
    run_test_case(test5, 0);

    return 0;
}