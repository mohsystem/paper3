#include <stdio.h>
#include <stdlib.h>

/**
 * Given an array of integers, create a new array with the smallest value removed.
 * Does not mutate the original array.
 * If there are multiple elements with the same value, remove the one with the lowest index.
 * If the input array is empty or has one element, the result is an empty array.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the input array.
 * @param newSize A pointer to an integer that will be populated with the size of the new array.
 * @return A pointer to a new, dynamically allocated array with the smallest element removed.
 *         Returns NULL if the resulting array is empty. The caller is responsible for freeing this memory.
 */
int* removeSmallest(const int* arr, int size, int* newSize) {
    if (arr == NULL || size <= 1) {
        *newSize = 0;
        return NULL;
    }

    int minIndex = 0;
    for (int i = 1; i < size; i++) {
        if (arr[i] < arr[minIndex]) {
            minIndex = i;
        }
    }

    *newSize = size - 1;
    int* result = (int*)malloc(*newSize * sizeof(int));
    if (result == NULL) {
        // Allocation failed
        *newSize = 0;
        return NULL;
    }

    int resultIndex = 0;
    for (int i = 0; i < size; i++) {
        if (i != minIndex) {
            result[resultIndex++] = arr[i];
        }
    }

    return result;
}

// Helper function to print an array
void printArray(const char* label, const int* arr, int size) {
    printf("%s", label);
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}

// Helper function to run a test case
void run_test(const int* arr, int size) {
    printArray("Input:  ", arr, size);
    int newSize;
    int* result = removeSmallest(arr, size, &newSize);
    printArray("Output: ", result, newSize);
    printf("---\n");
    free(result); // Free the dynamically allocated memory
}

int main() {
    int test1[] = {1, 2, 3, 4, 5};
    run_test(test1, 5);

    int test2[] = {5, 3, 2, 1, 4};
    run_test(test2, 5);
    
    int test3[] = {2, 2, 1, 2, 1};
    run_test(test3, 5);

    int test4[] = {10};
    run_test(test4, 1);

    int test5[] = {};
    run_test(test5, 0);

    return 0;
}