
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int key;
    int value;
} HashEntry;

int* twoSum(int* numbers, int numbersSize, int target, int* returnSize) {
    int* result = (int*)malloc(2 * sizeof(int));
    *returnSize = 2;
    
    for (int i = 0; i < numbersSize; i++) {
        for (int j = i + 1; j < numbersSize; j++) {
            if (numbers[i] + numbers[j] == target) {
                result[0] = i;
                result[1] = j;
                return result;
            }
        }
    }
    
    *returnSize = 0;
    return result;
}

int main() {
    int returnSize;
    
    // Test case 1
    int arr1[] = {1, 2, 3};
    int* result1 = twoSum(arr1, 3, 4, &returnSize);
    printf("Test 1: [%d, %d]\\n", result1[0], result1[1]);
    free(result1);
    
    // Test case 2
    int arr2[] = {3, 2, 4};
    int* result2 = twoSum(arr2, 3, 6, &returnSize);
    printf("Test 2: [%d, %d]\\n", result2[0], result2[1]);
    free(result2);
    
    // Test case 3
    int arr3[] = {1, 5, 3, 7, 9};
    int* result3 = twoSum(arr3, 5, 10, &returnSize);
    printf("Test 3: [%d, %d]\\n", result3[0], result3[1]);
    free(result3);
    
    // Test case 4
    int arr4[] = {10, 20, 30, 40};
    int* result4 = twoSum(arr4, 4, 50, &returnSize);
    printf("Test 4: [%d, %d]\\n", result4[0], result4[1]);
    free(result4);
    
    // Test case 5
    int arr5[] = {-1, -2, -3, -4, -5};
    int* result5 = twoSum(arr5, 5, -8, &returnSize);
    printf("Test 5: [%d, %d]\\n", result5[0], result5[1]);
    free(result5);
    
    return 0;
}
