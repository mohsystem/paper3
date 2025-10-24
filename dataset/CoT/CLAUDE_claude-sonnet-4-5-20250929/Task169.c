
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(int* nums, int* indices, int* result, int left, int mid, int right) {
    int size = right - left + 1;
    int* temp = (int*)malloc(size * sizeof(int));
    if (temp == NULL) return;
    
    int i = left, j = mid + 1, k = 0;
    int rightCount = 0;
    
    while (i <= mid && j <= right) {
        if (nums[indices[j]] < nums[indices[i]]) {
            temp[k++] = indices[j++];
            rightCount++;
        } else {
            result[indices[i]] += rightCount;
            temp[k++] = indices[i++];
        }
    }
    
    while (i <= mid) {
        result[indices[i]] += rightCount;
        temp[k++] = indices[i++];
    }
    
    while (j <= right) {
        temp[k++] = indices[j++];
    }
    
    memcpy(&indices[left], temp, size * sizeof(int));
    free(temp);
}

void mergeSort(int* nums, int* indices, int* result, int left, int right) {
    if (left >= right) return;
    
    int mid = left + (right - left) / 2;
    mergeSort(nums, indices, result, left, mid);
    mergeSort(nums, indices, result, mid + 1, right);
    merge(nums, indices, result, left, mid, right);
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }
    
    *returnSize = numsSize;
    int* result = (int*)calloc(numsSize, sizeof(int));
    int* indices = (int*)malloc(numsSize * sizeof(int));
    
    if (result == NULL || indices == NULL) {
        free(result);
        free(indices);
        *returnSize = 0;
        return NULL;
    }
    
    for (int i = 0; i < numsSize; i++) {
        indices[i] = i;
    }
    
    mergeSort(nums, indices, result, 0, numsSize - 1);
    free(indices);
    
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {5, 2, 6, 1};
    int returnSize1;
    int* result1 = countSmaller(nums1, 4, &returnSize1);
    printf("Test 1: ");
    for (int i = 0; i < returnSize1; i++) printf("%d ", result1[i]);
    printf("\\n");
    free(result1);
    
    // Test case 2
    int nums2[] = {-1};
    int returnSize2;
    int* result2 = countSmaller(nums2, 1, &returnSize2);
    printf("Test 2: ");
    for (int i = 0; i < returnSize2; i++) printf("%d ", result2[i]);
    printf("\\n");
    free(result2);
    
    // Test case 3
    int nums3[] = {-1, -1};
    int returnSize3;
    int* result3 = countSmaller(nums3, 2, &returnSize3);
    printf("Test 3: ");
    for (int i = 0; i < returnSize3; i++) printf("%d ", result3[i]);
    printf("\\n");
    free(result3);
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    int returnSize4;
    int* result4 = countSmaller(nums4, 5, &returnSize4);
    printf("Test 4: ");
    for (int i = 0; i < returnSize4; i++) printf("%d ", result4[i]);
    printf("\\n");
    free(result4);
    
    // Test case 5
    int nums5[] = {5, 4, 3, 2, 1};
    int returnSize5;
    int* result5 = countSmaller(nums5, 5, &returnSize5);
    printf("Test 5: ");
    for (int i = 0; i < returnSize5; i++) printf("%d ", result5[i]);
    printf("\\n");
    free(result5);
    
    return 0;
}
