
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Security: Structure to hold value and original index
typedef struct {
    int value;
    int index;
} Pair;

// Security: Input validation to prevent invalid array access
bool validateInput(const int* nums, int numsSize) {
    if (nums == NULL || numsSize < 1 || numsSize > 100000) {
        return false;
    }
    // Validate each element is within constraints
    for (int i = 0; i < numsSize; i++) {
        if (nums[i] < -10000 || nums[i] > 10000) {
            return false;
        }
    }
    return true;
}

// Security: Merge function with bounds checking
void merge(Pair* arr, int left, int mid, int right, int* counts) {
    // Security: Calculate size to prevent overflow
    int leftSize = mid - left + 1;
    int rightSize = right - mid;
    
    // Security: Check for potential overflow before allocation
    if (leftSize < 0 || rightSize < 0 || leftSize > 100000 || rightSize > 100000) {
        return;
    }
    
    // Security: Allocate temporary arrays with size validation
    Pair* leftArr = (Pair*)malloc(leftSize * sizeof(Pair));
    Pair* rightArr = (Pair*)malloc(rightSize * sizeof(Pair));
    
    // Security: Check malloc return values
    if (leftArr == NULL || rightArr == NULL) {
        free(leftArr);
        free(rightArr);
        return;
    }
    
    // Security: Use memcpy with validated sizes
    memcpy(leftArr, &arr[left], leftSize * sizeof(Pair));
    memcpy(rightArr, &arr[mid + 1], rightSize * sizeof(Pair));
    
    int i = 0, j = 0, k = left;
    int rightCount = 0;
    
    // Security: All loop bounds are validated
    while (i < leftSize && j < rightSize) {
        if (rightArr[j].value < leftArr[i].value) {
            rightCount++;
            arr[k++] = rightArr[j++];
        } else {
            counts[leftArr[i].index] += rightCount;
            arr[k++] = leftArr[i++];
        }
    }
    
    while (i < leftSize) {
        counts[leftArr[i].index] += rightCount;
        arr[k++] = leftArr[i++];
    }
    
    while (j < rightSize) {
        arr[k++] = rightArr[j++];
    }
    
    // Security: Free allocated memory
    free(leftArr);
    free(rightArr);
}

// Merge sort implementation
void mergeSort(Pair* arr, int left, int right, int* counts) {
    if (left >= right) return;
    
    // Security: Prevent integer overflow in midpoint calculation
    int mid = left + (right - left) / 2;
    
    mergeSort(arr, left, mid, counts);
    mergeSort(arr, mid + 1, right, counts);
    merge(arr, left, mid, right, counts);
}

// Security: Returns dynamically allocated array, caller must free
// returnSize is set to indicate the size of returned array
int* countSmaller(const int* nums, int numsSize, int* returnSize) {
    // Security: Initialize return size
    *returnSize = 0;
    
    // Security: Validate inputs
    if (!validateInput(nums, numsSize)) {
        return NULL;
    }
    
    // Security: Allocate counts array with size validation
    int* counts = (int*)calloc(numsSize, sizeof(int));
    if (counts == NULL) {
        return NULL;
    }
    
    // Security: Allocate pairs array with size validation
    Pair* pairs = (Pair*)malloc(numsSize * sizeof(Pair));
    if (pairs == NULL) {
        free(counts);
        return NULL;
    }
    
    // Security: Initialize pairs with bounds checking
    for (int i = 0; i < numsSize; i++) {
        pairs[i].value = nums[i];
        pairs[i].index = i;
    }
    
    // Perform merge sort
    mergeSort(pairs, 0, numsSize - 1, counts);
    
    // Security: Free temporary array
    free(pairs);
    
    *returnSize = numsSize;
    return counts;
}

int main() {
    // Test case 1
    int test1[] = {5, 2, 6, 1};
    int returnSize1 = 0;
    int* result1 = countSmaller(test1, 4, &returnSize1);
    printf("Test 1: [");
    if (result1 != NULL) {
        for (int i = 0; i < returnSize1; i++) {
            printf("%d", result1[i]);
            if (i < returnSize1 - 1) printf(",");
        }
        free(result1); // Security: Free allocated memory
    }
    printf("]\\n");
    
    // Test case 2
    int test2[] = {-1};
    int returnSize2 = 0;
    int* result2 = countSmaller(test2, 1, &returnSize2);
    printf("Test 2: [");
    if (result2 != NULL) {
        for (int i = 0; i < returnSize2; i++) {
            printf("%d", result2[i]);
            if (i < returnSize2 - 1) printf(",");
        }
        free(result2);
    }
    printf("]\\n");
    
    // Test case 3
    int test3[] = {-1, -1};
    int returnSize3 = 0;
    int* result3 = countSmaller(test3, 2, &returnSize3);
    printf("Test 3: [");
    if (result3 != NULL) {
        for (int i = 0; i < returnSize3; i++) {
            printf("%d", result3[i]);
            if (i < returnSize3 - 1) printf(",");
        }
        free(result3);
    }
    printf("]\\n");
    
    // Test case 4
    int test4[] = {1, 2, 3, 4, 5};
    int returnSize4 = 0;
    int* result4 = countSmaller(test4, 5, &returnSize4);
    printf("Test 4: [");
    if (result4 != NULL) {
        for (int i = 0; i < returnSize4; i++) {
            printf("%d", result4[i]);
            if (i < returnSize4 - 1) printf(",");
        }
        free(result4);
    }
    printf("]\\n");
    
    // Test case 5
    int test5[] = {5, 4, 3, 2, 1};
    int returnSize5 = 0;
    int* result5 = countSmaller(test5, 5, &returnSize5);
    printf("Test 5: [");
    if (result5 != NULL) {
        for (int i = 0; i < returnSize5; i++) {
            printf("%d", result5[i]);
            if (i < returnSize5 - 1) printf(",");
        }
        free(result5);
    }
    printf("]\\n");
    
    return 0;
}
