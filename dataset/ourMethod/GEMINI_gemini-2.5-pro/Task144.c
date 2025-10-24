#include <stdio.h>
#include <stdlib.h>

/**
 * Merges two sorted integer arrays into a single sorted array.
 * The caller is responsible for freeing the returned memory.
 *
 * @param arr1 The first sorted array. Can be NULL, treated as empty.
 * @param n1 The number of elements in arr1.
 * @param arr2 The second sorted array. Can be NULL, treated as empty.
 * @param n2 The number of elements in arr2.
 * @param merged_size A pointer to an integer where the size of the merged array will be stored.
 * @return A pointer to the newly allocated merged array, or NULL on allocation failure.
 */
int* mergeSortedArrays(const int* arr1, size_t n1, const int* arr2, size_t n2, size_t* merged_size) {
    if (arr1 == NULL) n1 = 0;
    if (arr2 == NULL) n2 = 0;
    
    size_t total_size = n1 + n2;
    *merged_size = total_size;

    if (total_size == 0) {
        return NULL; // Return NULL for a zero-sized array to be consistent with malloc
    }

    int* merged_array = (int*)malloc(total_size * sizeof(int));
    if (merged_array == NULL) {
        *merged_size = 0;
        perror("Failed to allocate memory");
        return NULL;
    }

    size_t i = 0, j = 0, k = 0;

    // Traverse both arrays and insert the smaller element into the merged array
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            merged_array[k++] = arr1[i++];
        } else {
            merged_array[k++] = arr2[j++];
        }
    }

    // Copy remaining elements of arr1, if any
    while (i < n1) {
        merged_array[k++] = arr1[i++];
    }

    // Copy remaining elements of arr2, if any
    while (j < n2) {
        merged_array[k++] = arr2[j++];
    }

    return merged_array;
}

void printArray(const char* label, const int* arr, size_t size) {
    printf("%s: [", label);
    if (arr != NULL) {
        for (size_t i = 0; i < size; ++i) {
            printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
        }
    }
    printf("]\n");
}

void run_test_case(int tc, const int* arr1, size_t n1, const int* arr2, size_t n2) {
    printf("Test Case %d:\n", tc);
    printArray("Input 1", arr1, n1);
    printArray("Input 2", arr2, n2);

    size_t merged_size;
    int* result = mergeSortedArrays(arr1, n1, arr2, n2, &merged_size);
    
    printArray("Output ", result, merged_size);
    
    free(result); // Free the allocated memory
    printf("\n");
}

int main() {
    // Test Case 1: General case
    int arr1_1[] = {1, 3, 5};
    int arr1_2[] = {2, 4, 6};
    run_test_case(1, arr1_1, 3, arr1_2, 3);
    
    // Test Case 2: One array is empty
    int arr2_1[] = {10, 20, 30};
    run_test_case(2, arr2_1, 3, NULL, 0);

    // Test Case 3: Both arrays are empty
    run_test_case(3, NULL, 0, NULL, 0);

    // Test Case 4: Arrays with different lengths
    int arr4_1[] = {1, 2, 9, 15};
    int arr4_2[] = {3, 10};
    run_test_case(4, arr4_1, 4, arr4_2, 2);

    // Test Case 5: Arrays with duplicate and negative numbers
    int arr5_1[] = {-5, 0, 5};
    int arr5_2[] = {-10, 0, 10};
    run_test_case(5, arr5_1, 3, arr5_2, 3);

    return 0;
}