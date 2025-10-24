
#include <stdio.h>
#include <stdlib.h>

int* mergeSortedArrays(int* arr1, int size1, int* arr2, int size2, int* resultSize) {
    *resultSize = size1 + size2;
    int* result = (int*)malloc(*resultSize * sizeof(int));
    
    if (result == NULL) {
        *resultSize = 0;
        return NULL;
    }
    
    int i = 0, j = 0, k = 0;
    
    while (i < size1 && j < size2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }
    
    while (i < size1) {
        result[k++] = arr1[i++];
    }
    
    while (j < size2) {
        result[k++] = arr2[j++];
    }
    
    return result;
}

int main() {
    // Test case 1: Normal case
    int arr1[] = {1, 3, 5, 7};
    int arr2[] = {2, 4, 6, 8};
    int resultSize;
    int* result1 = mergeSortedArrays(arr1, 4, arr2, 4, &resultSize);
    printf("Test 1: ");
    for (int i = 0; i < resultSize; i++) printf("%d ", result1[i]);
    printf("\\n");
    free(result1);
    
    // Test case 2: One empty array
    int arr3[] = {};
    int arr4[] = {1, 2, 3};
    int* result2 = mergeSortedArrays(arr3, 0, arr4, 3, &resultSize);
    printf("Test 2: ");
    for (int i = 0; i < resultSize; i++) printf("%d ", result2[i]);
    printf("\\n");
    free(result2);
    
    // Test case 3: Different lengths
    int arr5[] = {1, 5, 9};
    int arr6[] = {2, 3, 4, 6, 7, 8};
    int* result3 = mergeSortedArrays(arr5, 3, arr6, 6, &resultSize);
    printf("Test 3: ");
    for (int i = 0; i < resultSize; i++) printf("%d ", result3[i]);
    printf("\\n");
    free(result3);
    
    // Test case 4: Duplicate elements
    int arr7[] = {1, 3, 5, 5};
    int arr8[] = {2, 3, 5, 6};
    int* result4 = mergeSortedArrays(arr7, 4, arr8, 4, &resultSize);
    printf("Test 4: ");
    for (int i = 0; i < resultSize; i++) printf("%d ", result4[i]);
    printf("\\n");
    free(result4);
    
    // Test case 5: Negative numbers
    int arr9[] = {-5, -3, 0, 2};
    int arr10[] = {-4, -1, 1, 3};
    int* result5 = mergeSortedArrays(arr9, 4, arr10, 4, &resultSize);
    printf("Test 5: ");
    for (int i = 0; i < resultSize; i++) printf("%d ", result5[i]);
    printf("\\n");
    free(result5);
    
    return 0;
}
