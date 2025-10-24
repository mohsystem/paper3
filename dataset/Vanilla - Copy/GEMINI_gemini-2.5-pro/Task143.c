#include <stdio.h>
#include <stdlib.h>

/**
 * Comparison function for qsort.
 */
int compare(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);

    if (int_a == int_b) return 0;
    else if (int_a < int_b) return -1;
    else return 1;
}

/**
 * Sorts an array of integers in ascending order in-place.
 *
 * @param arr The array of integers to sort.
 * @param n The number of elements in the array.
 */
void sortArray(int* arr, size_t n) {
    if (arr == NULL) {
        return;
    }
    qsort(arr, n, sizeof(int), compare);
}

/**
 * Helper function to print an array of integers.
 * @param label A description for the output.
 * @param arr The array to print.
 * @param n The number of elements in the array.
 */
void printArray(const char* label, const int* arr, size_t n) {
    printf("%s", label);
    for (size_t i = 0; i < n; i++) {
        printf("%d%s", arr[i], (i == n - 1) ? "" : " ");
    }
    printf("\n");
}

int main() {
    // Test Case 1: Standard unsorted array
    int arr1[] = {5, 2, 8, 1, 9};
    size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1\n");
    printArray("Original: ", arr1, n1);
    sortArray(arr1, n1);
    printArray("Sorted:   ", arr1, n1);
    printf("\n");

    // Test Case 2: Reverse-sorted array
    int arr2[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    size_t n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2\n");
    printArray("Original: ", arr2, n2);
    sortArray(arr2, n2);
    printArray("Sorted:   ", arr2, n2);
    printf("\n");

    // Test Case 3: Already sorted array
    int arr3[] = {1, 2, 3, 4, 5};
    size_t n3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3\n");
    printArray("Original: ", arr3, n3);
    sortArray(arr3, n3);
    printArray("Sorted:   ", arr3, n3);
    printf("\n");

    // Test Case 4: Array with duplicate elements
    int arr4[] = {4, 2, 7, 2, 5, 4};
    size_t n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4\n");
    printArray("Original: ", arr4, n4);
    sortArray(arr4, n4);
    printArray("Sorted:   ", arr4, n4);
    printf("\n");

    // Test Case 5: Array with negative numbers and zero
    int arr5[] = {-5, 0, 10, -2, 3};
    size_t n5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5\n");
    printArray("Original: ", arr5, n5);
    sortArray(arr5, n5);
    printArray("Sorted:   ", arr5, n5);
    printf("\n");

    return 0;
}