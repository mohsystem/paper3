#include <stdio.h>
#include <stdlib.h>
#include <string.h> // For memcpy
#include <stdint.h> // For SIZE_MAX

/**
 * Merges two sorted integer arrays into a new sorted array.
 * This function is secure against null inputs, integer overflow for the combined size,
 * and memory allocation failures.
 *
 * @param arr1 Pointer to the first sorted array.
 * @param size1 Number of elements in arr1.
 * @param arr2 Pointer to the second sorted array.
 * @param size2 Number of elements in arr2.
 * @param merged_size Pointer to a size_t variable where the size of the merged array will be stored.
 * @return A pointer to the newly allocated merged array, or NULL on failure.
 *         The caller is responsible for freeing the returned memory using free().
 */
int* mergeSortedArrays(const int* arr1, size_t size1, const int* arr2, size_t size2, size_t* merged_size) {
    // Security: Validate arguments. If an array is NULL, its size must be 0.
    if ((arr1 == NULL && size1 != 0) || (arr2 == NULL && size2 != 0) || merged_size == NULL) {
        if (merged_size) *merged_size = 0;
        return NULL;
    }
    
    // Security: Check for size_t overflow before calculating total size
    if (size1 > SIZE_MAX - size2) {
        fprintf(stderr, "Error: Combined array size is too large, causing overflow.\n");
        *merged_size = 0;
        return NULL;
    }
    
    size_t total_size = size1 + size2;
    *merged_size = total_size;

    if (total_size == 0) {
        return NULL; // Return NULL for a zero-sized array to avoid malloc(0) ambiguity
    }

    // Security: Allocate memory and check if allocation succeeded
    int* merged = (int*)malloc(total_size * sizeof(int));
    if (merged == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        *merged_size = 0;
        return NULL;
    }

    size_t i = 0, j = 0, k = 0;

    // Traverse both arrays and insert the smaller element into the merged array
    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            merged[k++] = arr1[i++];
        } else {
            merged[k++] = arr2[j++];
        }
    }

    // Copy remaining elements from arr1, if any, using memcpy for efficiency
    if (i < size1) {
        memcpy(merged + k, arr1 + i, (size1 - i) * sizeof(int));
    }
    
    // Copy remaining elements from arr2, if any
    if (j < size2) {
        memcpy(merged + k, arr2 + j, (size2 - j) * sizeof(int));
    }

    return merged;
}

// Helper function to print an array for testing
void printArray(const char* label, const int* arr, size_t size) {
    printf("%s", label);
    if (arr == NULL && size > 0) { // Should not happen with valid inputs
        printf("Invalid Array (NULL with size > 0)\n");
        return;
    }
    printf("[");
    for (size_t i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]\n");
}

// Helper function to run a test case and manage memory
void run_test_case(const char* name, int* arr1, size_t size1, int* arr2, size_t size2) {
    printf("%s\n", name);
    printArray("Array 1: ", arr1, size1);
    printArray("Array 2: ", arr2, size2);
    size_t merged_size;
    int* merged_array = mergeSortedArrays(arr1, size1, arr2, size2, &merged_size);
    printArray("Merged:  ", merged_array, merged_size);
    printf("\n");
    free(merged_array); // IMPORTANT: Free the allocated memory
}

int main() {
    // Test Case 1: Standard merge
    int arr1_1[] = {1, 3, 5, 7};
    int arr1_2[] = {2, 4, 6, 8};
    run_test_case("Test Case 1:", arr1_1, 4, arr1_2, 4);

    // Test Case 2: One array is empty
    int arr2_1[] = {10, 20, 30};
    run_test_case("Test Case 2:", arr2_1, 3, NULL, 0);

    // Test Case 3: First array is empty
    int arr3_2[] = {5, 15, 25};
    run_test_case("Test Case 3:", NULL, 0, arr3_2, 3);

    // Test Case 4: Arrays with duplicate values and different lengths
    int arr4_1[] = {1, 2, 2, 5, 9};
    int arr4_2[] = {2, 6, 10};
    run_test_case("Test Case 4:", arr4_1, 5, arr4_2, 3);
    
    // Test Case 5: Both arrays are empty
    run_test_case("Test Case 5:", NULL, 0, NULL, 0);

    return 0;
}