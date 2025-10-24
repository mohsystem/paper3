
#include <stdio.h>
#include <stdlib.h>

int mergeSort(int* nums, int left, int right);
void merge(int* nums, int left, int mid, int right);

int reversePairs(int* nums, int numsSize) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    return mergeSort(nums, 0, numsSize - 1);
}

int mergeSort(int* nums, int left, int right) {
    if (left >= right) {
        return 0;
    }
    
    int mid = left + (right - left) / 2;
    int count = mergeSort(nums, left, mid) + mergeSort(nums, mid + 1, right);
    
    // Count reverse pairs
    int j = mid + 1;
    for (int i = left; i <= mid; i++) {
        while (j <= right && nums[i] > 2LL * nums[j]) {
            j++;
        }
        count += j - (mid + 1);
    }
    
    // Merge
    merge(nums, left, mid, right);
    return count;
}

void merge(int* nums, int left, int mid, int right) {
    int size = right - left + 1;
    int* temp = (int*)malloc(size * sizeof(int));
    int i = left, j = mid + 1, k = 0;
    
    while (i <= mid && j <= right) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    while (i <= mid) {
        temp[k++] = nums[i++];
    }
    
    while (j <= right) {
        temp[k++] = nums[j++];
    }
    
    for (i = 0; i < size; i++) {
        nums[left + i] = temp[i];
    }
    
    free(temp);
}

int main() {
    // Test case 1
    int nums1[] = {1, 3, 2, 3, 1};
    printf("Test 1: %d\\n", reversePairs(nums1, 5)); // Expected: 2
    
    // Test case 2
    int nums2[] = {2, 4, 3, 5, 1};
    printf("Test 2: %d\\n", reversePairs(nums2, 5)); // Expected: 3
    
    // Test case 3
    int nums3[] = {5, 4, 3, 2, 1};
    printf("Test 3: %d\\n", reversePairs(nums3, 5)); // Expected: 4
    
    // Test case 4
    int nums4[] = {1, 2, 3, 4, 5};
    printf("Test 4: %d\\n", reversePairs(nums4, 5)); // Expected: 0
    
    // Test case 5
    int nums5[] = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    printf("Test 5: %d\\n", reversePairs(nums5, 6)); // Expected: 0
    
    return 0;
}
