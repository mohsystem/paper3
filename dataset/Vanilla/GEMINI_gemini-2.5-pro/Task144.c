#include <stdio.h>
#include <stdlib.h>

/**
 * Merges two sorted integer arrays into a single sorted array.
 *
 * @param arr1 The first sorted integer array.
 * @param n1 The size of the first array.
 * @param arr2 The second sorted integer array.
 * @param n2 The size of the second array.
 * @return A new dynamically allocated array containing all elements from arr1 and arr2 in sorted order.
 *         The caller is responsible for freeing this memory.
 */
int* mergeArrays(const int* arr1, int n1, const int* arr2, int n2) {
    int* result = (int*)malloc((n1 + n2) * sizeof(int));
    if (result == NULL) {
        printf("Memory allocation failed\n");
        exit(1);
    }

    int i = 0, j = 0, k = 0;

    // Traverse both arrays and insert the smaller element into the result
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }

    // Copy remaining elements of arr1, if any
    while (i < n1) {
        result[k++] = arr1[i++];
    }

    // Copy remaining elements of arr2, if any
    while (j < n2) {
        result[k++] = arr2[j++];
    }

    return result;
}

void printArray(const char* label, const int* arr, int size) {
    printf("%s[", label);
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    int arr1_1[] = {1, 3, 5};
    int arr1_2[] = {2, 4, 6};
    int n1_1 = sizeof(arr1_1) / sizeof(arr1_1[0]);
    int n1_2 = sizeof(arr1_2) / sizeof(arr1_2[0]);
    int* res1 = mergeArrays(arr1_1, n1_1, arr1_2, n1_2);
    printArray("Test Case 1: ", res1, n1_1 + n1_2);
    free(res1);

    // Test Case 2
    int arr2_1[] = {10, 20, 30};
    int n2_1 = sizeof(arr2_1) / sizeof(arr2_1[0]);
    int n2_2 = 0;
    int* res2 = mergeArrays(arr2_1, n2_1, NULL, n2_2);
    printArray("Test Case 2: ", res2, n2_1 + n2_2);
    free(res2);
    
    // Test Case 3
    int arr3_2[] = {5, 15, 25};
    int n3_1 = 0;
    int n3_2 = sizeof(arr3_2) / sizeof(arr3_2[0]);
    int* res3 = mergeArrays(NULL, n3_1, arr3_2, n3_2);
    printArray("Test Case 3: ", res3, n3_1 + n3_2);
    free(res3);

    // Test Case 4
    int arr4_1[] = {1, 5, 9, 10};
    int arr4_2[] = {2, 3, 8};
    int n4_1 = sizeof(arr4_1) / sizeof(arr4_1[0]);
    int n4_2 = sizeof(arr4_2) / sizeof(arr4_2[0]);
    int* res4 = mergeArrays(arr4_1, n4_1, arr4_2, n4_2);
    printArray("Test Case 4: ", res4, n4_1 + n4_2);
    free(res4);

    // Test Case 5
    int arr5_1[] = {1, 2, 3};
    int arr5_2[] = {4, 5, 6};
    int n5_1 = sizeof(arr5_1) / sizeof(arr5_1[0]);
    int n5_2 = sizeof(arr5_2) / sizeof(arr5_2[0]);
    int* res5 = mergeArrays(arr5_1, n5_1, arr5_2, n5_2);
    printArray("Test Case 5: ", res5, n5_1 + n5_2);
    free(res5);
    
    return 0;
}