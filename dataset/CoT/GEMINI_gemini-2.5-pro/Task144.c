#include <stdio.h>
#include <stdlib.h>

/**
 * Merges two sorted integer arrays into a single sorted array.
 * The caller is responsible for freeing the returned array.
 *
 * @param arr1 The first sorted integer array.
 * @param n1 The size of the first array.
 * @param arr2 The second sorted integer array.
 * @param n2 The size of the second array.
 * @param mergedSize A pointer to an integer to store the size of the merged array.
 * @return A pointer to the newly allocated sorted array, or NULL on failure.
 */
int* mergeArrays(const int* arr1, int n1, const int* arr2, int n2, int* mergedSize) {
    *mergedSize = n1 + n2;
    if (*mergedSize == 0) {
        return NULL; // Or return a valid pointer to 0-size block if preferred
    }

    int* mergedArray = (int*)malloc(*mergedSize * sizeof(int));
    if (mergedArray == NULL) {
        // Failed to allocate memory
        *mergedSize = 0;
        return NULL;
    }

    int i = 0; // Pointer for arr1
    int j = 0; // Pointer for arr2
    int k = 0; // Pointer for mergedArray

    // Traverse both arrays and insert the smaller element into the merged array
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            mergedArray[k++] = arr1[i++];
        } else {
            mergedArray[k++] = arr2[j++];
        }
    }

    // Copy remaining elements of arr1, if any
    while (i < n1) {
        mergedArray[k++] = arr1[i++];
    }

    // Copy remaining elements of arr2, if any
    while (j < n2) {
        mergedArray[k++] = arr2[j++];
    }

    return mergedArray;
}

// Helper function to print an array
void printArray(const char* name, const int* arr, int size) {
    printf("%s", name);
    if (size == 0 || arr == NULL) {
        printf("(empty)\n");
        return;
    }
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    int mergedSize;
    int* merged;

    // Test Case 1: Standard merge
    int arr1[] = {1, 3, 5, 7};
    int arr2[] = {2, 4, 6, 8};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 1:\n");
    printArray("Array 1: ", arr1, n1);
    printArray("Array 2: ", arr2, n2);
    merged = mergeArrays(arr1, n1, arr2, n2, &mergedSize);
    printArray("Merged:  ", merged, mergedSize);
    free(merged); // Free the allocated memory
    printf("\n");

    // Test Case 2: Arrays with different lengths
    int arr3[] = {10, 20, 30};
    int arr4[] = {5, 15, 25, 35, 45};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    int n4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 2:\n");
    printArray("Array 1: ", arr3, n3);
    printArray("Array 2: ", arr4, n4);
    merged = mergeArrays(arr3, n3, arr4, n4, &mergedSize);
    printArray("Merged:  ", merged, mergedSize);
    free(merged);
    printf("\n");

    // Test Case 3: One array is empty
    int arr5[] = {1, 2, 3};
    int arr6[] = {};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);
    int n6 = sizeof(arr6) / sizeof(arr6[0]);
    printf("Test Case 3:\n");
    printArray("Array 1: ", arr5, n5);
    printArray("Array 2: ", arr6, n6);
    merged = mergeArrays(arr5, n5, arr6, n6, &mergedSize);
    printArray("Merged:  ", merged, mergedSize);
    free(merged);
    printf("\n");

    // Test Case 4: Both arrays are empty
    int arr7[] = {};
    int arr8[] = {};
    int n7 = sizeof(arr7) / sizeof(arr7[0]);
    int n8 = sizeof(arr8) / sizeof(arr8[0]);
    printf("Test Case 4:\n");
    printArray("Array 1: ", arr7, n7);
    printArray("Array 2: ", arr8, n8);
    merged = mergeArrays(arr7, n7, arr8, n8, &mergedSize);
    printArray("Merged:  ", merged, mergedSize);
    free(merged); // Merged will be NULL, free(NULL) is safe.
    printf("\n");

    // Test Case 5: Arrays with negative numbers and duplicates
    int arr9[] = {-5, 0, 5, 5, 15};
    int arr10[] = {-10, -5, 10, 20};
    int n9 = sizeof(arr9) / sizeof(arr9[0]);
    int n10 = sizeof(arr10) / sizeof(arr10[0]);
    printf("Test Case 5:\n");
    printArray("Array 1: ", arr9, n9);
    printArray("Array 2: ", arr10, n10);
    merged = mergeArrays(arr9, n9, arr10, n10, &mergedSize);
    printArray("Merged:  ", merged, mergedSize);
    free(merged);
    printf("\n");
    
    return 0;
}