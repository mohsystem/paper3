#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Comparison function for qsort.
 * This function is used to determine the order of the elements.
 * It returns a negative, zero, or positive value depending on whether
 * the first argument is less than, equal to, or greater than the second.
 */
int compare_integers(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);

    if (int_a < int_b) return -1;
    if (int_a > int_b) return 1;
    return 0;
}

/**
 * @brief Sorts an array of integers in ascending order.
 * 
 * @param arr The array of integers to be sorted.
 * @param size The number of elements in the array.
 */
void sortArray(int arr[], size_t size) {
    // Check for NULL pointer or zero size to prevent undefined behavior.
    if (arr == NULL || size == 0) {
        return;
    }
    qsort(arr, size, sizeof(int), compare_integers);
}

/**
 * @brief Helper function to print an array of integers.
 * 
 * @param arr The array to print.
 * @param size The number of elements in the array.
 */
void printArray(const int arr[], size_t size) {
    printf("[");
    for (size_t i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1: General case
    int arr1[] = {5, 2, 8, 1, 9, 4};
    size_t size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1:\n");
    printf("Original: ");
    printArray(arr1, size1);
    printf("\n");
    sortArray(arr1, size1);
    printf("Sorted:   ");
    printArray(arr1, size1);
    printf("\n\n");

    // Test Case 2: Already sorted
    int arr2[] = {1, 2, 3, 4, 5, 6};
    size_t size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2:\n");
    printf("Original: ");
    printArray(arr2, size2);
    printf("\n");
    sortArray(arr2, size2);
    printf("Sorted:   ");
    printArray(arr2, size2);
    printf("\n\n");

    // Test Case 3: Reverse sorted
    int arr3[] = {10, 8, 6, 4, 2, 0};
    size_t size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3:\n");
    printf("Original: ");
    printArray(arr3, size3);
    printf("\n");
    sortArray(arr3, size3);
    printf("Sorted:   ");
    printArray(arr3, size3);
    printf("\n\n");

    // Test Case 4: With duplicates and negative numbers
    int arr4[] = {4, -1, 5, 2, 4, -1, 0};
    size_t size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4:\n");
    printf("Original: ");
    printArray(arr4, size4);
    printf("\n");
    sortArray(arr4, size4);
    printf("Sorted:   ");
    printArray(arr4, size4);
    printf("\n\n");
    
    // Test Case 5: Empty array
    int arr5[] = {};
    size_t size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5:\n");
    printf("Original: ");
    printArray(arr5, size5);
    printf("\n");
    sortArray(arr5, size5);
    printf("Sorted:   ");
    printArray(arr5, size5);
    printf("\n\n");

    return 0;
}