#include <stdio.h>
#include <stdlib.h> // For qsort

/**
 * Secure comparison function for qsort.
 * It avoids subtraction which can lead to integer overflow
 * on certain platforms or with specific values.
 */
static int compare_integers(const void* a, const void* b) {
    int val1 = *(const int*)a;
    int val2 = *(const int*)b;
    if (val1 < val2) {
        return -1;
    } else if (val1 > val2) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * Sorts an array of integers in ascending order.
 * This function sorts the array in-place.
 *
 * @param arr The array of integers to be sorted.
 * @param n The number of elements in the array.
 */
void sortArray(int arr[], size_t n) {
    // Secure: Check for null pointer to prevent crashes.
    if (arr == NULL) {
        fprintf(stderr, "Input array cannot be null.\n");
        return;
    }
    // Secure: Use the standard library's qsort, which is a well-tested
    // and generally efficient implementation.
    qsort(arr, n, sizeof(int), compare_integers);
}

/**
 * Helper function to print an array of integers.
 */
void printArray(const int arr[], size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("%d", arr[i]);
        if (i < n - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1: Regular unsorted array
    printf("--- Test Case 1: Regular unsorted array ---\n");
    int test1[] = {5, 2, 8, 1, 9, 4};
    size_t n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Before: ");
    printArray(test1, n1);
    printf("\n");
    sortArray(test1, n1);
    printf("After:  ");
    printArray(test1, n1);
    printf("\n\n");

    // Test Case 2: Array with negative numbers and duplicates
    printf("--- Test Case 2: Array with negative numbers and duplicates ---\n");
    int test2[] = {-5, 2, -8, 2, 9, 4, -5};
    size_t n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Before: ");
    printArray(test2, n2);
    printf("\n");
    sortArray(test2, n2);
    printf("After:  ");
    printArray(test2, n2);
    printf("\n\n");

    // Test Case 3: Already sorted array
    printf("--- Test Case 3: Already sorted array ---\n");
    int test3[] = {1, 2, 3, 4, 5, 6};
    size_t n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Before: ");
    printArray(test3, n3);
    printf("\n");
    sortArray(test3, n3);
    printf("After:  ");
    printArray(test3, n3);
    printf("\n\n");

    // Test Case 4: Reverse sorted array
    printf("--- Test Case 4: Reverse sorted array ---\n");
    int test4[] = {10, 8, 6, 4, 2, 0};
    size_t n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Before: ");
    printArray(test4, n4);
    printf("\n");
    sortArray(test4, n4);
    printf("After:  ");
    printArray(test4, n4);
    printf("\n\n");

    // Test Case 5: Empty array and null array
    printf("--- Test Case 5: Edge cases (empty and null) ---\n");
    int test5_empty[] = {};
    size_t n5_empty = 0;
    printf("Before (empty): ");
    printArray(test5_empty, n5_empty);
    printf("\n");
    sortArray(test5_empty, n5_empty);
    printf("After (empty):  ");
    printArray(test5_empty, n5_empty);
    printf("\n");

    int* test5_null = NULL;
    printf("Before (null): null\n");
    sortArray(test5_null, 0); // The function will print an error message
    printf("\n");

    return 0;
}