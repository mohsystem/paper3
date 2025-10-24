
#include <stdio.h>
#include <stdlib.h>

int* mergeSortedArrays(int* arr1, int n1, int* arr2, int n2, int* resultSize) {
    *resultSize = n1 + n2;
    int* result = (int*)malloc(*resultSize * sizeof(int));
    
    int i = 0, j = 0, k = 0;
    
    while (i < n1 && j < n2) {
        if (arr1[i] <= arr2[j]) {
            result[k++] = arr1[i++];
        } else {
            result[k++] = arr2[j++];
        }
    }
    
    while (i < n1) {
        result[k++] = arr1[i++];
    }
    
    while (j < n2) {
        result[k++] = arr2[j++];
    }
    
    return result;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]\\n");
}

int main() {
    int resultSize;
    
    // Test case 1
    int arr1_1[] = {1, 3, 5, 7};
    int arr2_1[] = {2, 4, 6, 8};
    int* result1 = mergeSortedArrays(arr1_1, 4, arr2_1, 4, &resultSize);
    printf("Test 1: ");
    printArray(result1, resultSize);
    free(result1);
    
    // Test case 2
    int arr1_2[] = {1, 2, 3};
    int arr2_2[] = {4, 5, 6};
    int* result2 = mergeSortedArrays(arr1_2, 3, arr2_2, 3, &resultSize);
    printf("Test 2: ");
    printArray(result2, resultSize);
    free(result2);
    
    // Test case 3
    int arr1_3[] = {1, 5, 9};
    int arr2_3[] = {2, 3, 4, 6, 7, 8};
    int* result3 = mergeSortedArrays(arr1_3, 3, arr2_3, 6, &resultSize);
    printf("Test 3: ");
    printArray(result3, resultSize);
    free(result3);
    
    // Test case 4
    int arr1_4[] = {};
    int arr2_4[] = {1, 2, 3};
    int* result4 = mergeSortedArrays(arr1_4, 0, arr2_4, 3, &resultSize);
    printf("Test 4: ");
    printArray(result4, resultSize);
    free(result4);
    
    // Test case 5
    int arr1_5[] = {-5, -2, 0, 3};
    int arr2_5[] = {-3, -1, 2, 4, 6};
    int* result5 = mergeSortedArrays(arr1_5, 4, arr2_5, 5, &resultSize);
    printf("Test 5: ");
    printArray(result5, resultSize);
    free(result5);
    
    return 0;
}
