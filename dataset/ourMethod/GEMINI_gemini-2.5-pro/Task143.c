#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Comparison function for qsort.
 * Compares two integers for ascending order.
 *
 * @param a Pointer to the first element.
 * @param b Pointer to the second element.
 * @return int < 0 if a < b, 0 if a == b, > 0 if a > b.
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
 * The sorting is done in-place.
 *
 * @param arr Pointer to the integer array.
 * @param count The number of elements in the array.
 */
void sortArray(int* arr, size_t count) {
    // Rule #3: Validate input. Check for null pointer.
    if (arr == NULL) {
        return;
    }
    // An empty array (count == 0) is valid and qsort handles it correctly.
    
    // qsort is the standard C library sort function. It is memory-safe
    // when provided with correct arguments (base, count, size, comparator),
    // adhering to Rule #4.
    qsort(arr, count, sizeof(int), compare_integers);
}

/**
 * @brief Prints the elements of an integer array.
 * @param arr Pointer to the array.
 * @param count Number of elements in the array.
 */
void printArray(const int* arr, size_t count) {
    printf("[");
    for (size_t i = 0; i < count; ++i) {
        printf("%d%s", arr[i], (i == count - 1) ? "" : ", ");
    }
    printf("]");
}

/**
 * @brief Main function for testing the sortArray function.
 * @return int Exit code.
 */
int main(void) {
    // Test Case 1: Empty array
    int arr1[] = {};
    size_t count1 = sizeof(arr1) / sizeof(arr1[0]);
    if (count1 == 0 && sizeof(arr1) == 0) { // Handle C++ zero-size array extension
        count1 = 0;
    }


    // Test Case 2: Mixed positive and negative numbers
    int arr2[] = {5, -2, 9, 1, 5, 6};
    size_t count2 = sizeof(arr2) / sizeof(arr2[0]);

    // Test Case 3: Single element array
    int arr3[] = {1};
    size_t count3 = sizeof(arr3) / sizeof(arr3[0]);

    // Test Case 4: Reverse sorted array
    int arr4[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    size_t count4 = sizeof(arr4) / sizeof(arr4[0]);

    // Test Case 5: Already sorted array
    int arr5[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    size_t count5 = sizeof(arr5) / sizeof(arr5[0]);

    // Running test cases
    printf("Test Case 1:\n");
    printf("Original array: "); printArray(arr1, count1); printf("\n");
    sortArray(arr1, count1);
    printf("Sorted array:   "); printArray(arr1, count1); printf("\n\n");

    printf("Test Case 2:\n");
    printf("Original array: "); printArray(arr2, count2); printf("\n");
    sortArray(arr2, count2);
    printf("Sorted array:   "); printArray(arr2, count2); printf("\n\n");

    printf("Test Case 3:\n");
    printf("Original array: "); printArray(arr3, count3); printf("\n");
    sortArray(arr3, count3);
    printf("Sorted array:   "); printArray(arr3, count3); printf("\n\n");

    printf("Test Case 4:\n");
    printf("Original array: "); printArray(arr4, count4); printf("\n");
    sortArray(arr4, count4);
    printf("Sorted array:   "); printArray(arr4, count4); printf("\n\n");

    printf("Test Case 5:\n");
    printf("Original array: "); printArray(arr5, count5); printf("\n");
    sortArray(arr5, count5);
    printf("Sorted array:   "); printArray(arr5, count5); printf("\n\n");

    return 0;
}