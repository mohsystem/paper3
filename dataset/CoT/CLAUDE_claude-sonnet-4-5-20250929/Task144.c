
#include <stdio.h>
#include <stdlib.h>

int* mergeSortedArrays(const int* arr1, int len1, const int* arr2, int len2, int* resultLen) {
    // Input validation
    if (arr1 == NULL) len1 = 0;
    if (arr2 == NULL) len2 = 0;
    
    *resultLen = len1 + len2;
    int* result = (int*)malloc((*resultLen) * sizeof(int));
    
    if (result == NULL) {
        *resultLen = 0;
        return NULL;
    }
    
    int i = 0, j = 0, k = 0;
    
    // Merge arrays while both have elements
    while (i < len1 && j < len2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }
    
    // Copy remaining elements from arr1
    while (i < len1) {
        result[k++] = arr1[i++];
    }
    
    // Copy remaining elements from arr2
    while (j < len2) {
        result[k++] = arr2[j++];
    }
    
    return result;
}

void printArray(const int* arr, int len) {
    for (int i = 0; i < len; i++) {
        printf("%d ", arr[i]);
    }
    printf("\\n");
}

int main() {
    int resultLen;
    
    // Test case 1: Normal case
    int arr1[] = {1, 3, 5, 7};
    int arr2[] = {2, 4, 6, 8};
    int* result1 = mergeSortedArrays(arr1, 4, arr2, 4, &resultLen);
    printf("Test 1: ");
    printArray(result1, resultLen);
    free(result1);
    
    // Test case 2: One empty array
    int arr4[] = {1, 2, 3};
    int* result2 = mergeSortedArrays(NULL, 0, arr4, 3, &resultLen);
    printf("Test 2: ");
    printArray(result2, resultLen);
    free(result2);
    
    // Test case 3: Arrays with duplicates
    int arr5[] = {1, 3, 3, 5};
    int arr6[] = {2, 3, 4};
    int* result3 = mergeSortedArrays(arr5, 4, arr6, 3, &resultLen);
    printf("Test 3: ");
    printArray(result3, resultLen);
    free(result3);
    
    // Test case 4: Different lengths
    int arr7[] = {1, 2};
    int arr8[] = {3, 4, 5, 6, 7};
    int* result4 = mergeSortedArrays(arr7, 2, arr8, 5, &resultLen);
    printf("Test 4: ");
    printArray(result4, resultLen);
    free(result4);
    
    // Test case 5: Negative numbers
    int arr9[] = {-5, -3, -1};
    int arr10[] = {-4, -2, 0, 2};
    int* result5 = mergeSortedArrays(arr9, 3, arr10, 4, &resultLen);
    printf("Test 5: ");
    printArray(result5, resultLen);
    free(result5);
    
    return 0;
}
