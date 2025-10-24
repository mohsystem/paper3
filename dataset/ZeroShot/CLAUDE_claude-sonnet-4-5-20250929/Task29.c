
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* indices;
    int size;
} Result;

Result twoSum(int* numbers, int numbersSize, int target) {
    Result result;
    result.indices = (int*)malloc(2 * sizeof(int));
    result.size = 0;
    
    if (numbers == NULL || numbersSize < 2) {
        return result;
    }
    
    for (int i = 0; i < numbersSize - 1; i++) {
        for (int j = i + 1; j < numbersSize; j++) {
            if (numbers[i] + numbers[j] == target) {
                result.indices[0] = i;
                result.indices[1] = j;
                result.size = 2;
                return result;
            }
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    int arr1[] = {1, 2, 3};
    Result result1 = twoSum(arr1, 3, 4);
    printf("Test 1: [%d, %d]\\n", result1.indices[0], result1.indices[1]);
    free(result1.indices);
    
    // Test case 2
    int arr2[] = {3, 2, 4};
    Result result2 = twoSum(arr2, 3, 6);
    printf("Test 2: [%d, %d]\\n", result2.indices[0], result2.indices[1]);
    free(result2.indices);
    
    // Test case 3
    int arr3[] = {1, 5, 3, 7, 9};
    Result result3 = twoSum(arr3, 5, 10);
    printf("Test 3: [%d, %d]\\n", result3.indices[0], result3.indices[1]);
    free(result3.indices);
    
    // Test case 4
    int arr4[] = {10, 20, 30, 40};
    Result result4 = twoSum(arr4, 4, 70);
    printf("Test 4: [%d, %d]\\n", result4.indices[0], result4.indices[1]);
    free(result4.indices);
    
    // Test case 5
    int arr5[] = {-1, -2, -3, -4, -5};
    Result result5 = twoSum(arr5, 5, -8);
    printf("Test 5: [%d, %d]\\n", result5.indices[0], result5.indices[1]);
    free(result5.indices);
    
    return 0;
}
