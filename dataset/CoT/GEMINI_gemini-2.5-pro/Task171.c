#include <stdio.h>
#include <stdlib.h>

int merge(int* nums, int* temp, int low, int mid, int high);
int mergeSort(int* nums, int* temp, int low, int high);

int reversePairs(int* nums, int numsSize) {
    if (nums == NULL || numsSize < 2) {
        return 0;
    }
    int* temp = (int*)malloc(sizeof(int) * numsSize);
    if (temp == NULL) {
        return -1; // Memory allocation failed
    }
    int count = mergeSort(nums, temp, 0, numsSize - 1);
    free(temp);
    return count;
}

int mergeSort(int* nums, int* temp, int low, int high) {
    if (low >= high) {
        return 0;
    }
    int mid = low + (high - low) / 2;
    int count = mergeSort(nums, temp, low, mid);
    count += mergeSort(nums, temp, mid + 1, high);
    count += merge(nums, temp, low, mid, high);
    return count;
}

int merge(int* nums, int* temp, int low, int mid, int high) {
    int count = 0;
    int j = mid + 1;
    for (int i = low; i <= mid; i++) {
        while (j <= high && (long long)nums[i] > 2LL * nums[j]) {
            j++;
        }
        count += (j - (mid + 1));
    }

    int i = low;
    j = mid + 1;
    int k = low;
    
    while (i <= mid && j <= high) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }
    
    while (i <= mid) {
        temp[k++] = nums[i++];
    }
    
    while (j <= high) {
        temp[k++] = nums[j++];
    }
    
    for (int l = low; l <= high; l++) {
        nums[l] = temp[l];
    }
    
    return count;
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, 2, 3, 1};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Test Case 1: %d\n", reversePairs(nums1, size1)); // Expected: 2

    // Test Case 2
    int nums2[] = {2, 4, 3, 5, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("Test Case 2: %d\n", reversePairs(nums2, size2)); // Expected: 3

    // Test Case 3
    int nums3[] = {5, 4, 3, 2, 1};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Test Case 3: %d\n", reversePairs(nums3, size3)); // Expected: 4

    // Test Case 4
    int nums4[] = {2147483647, 1000000000};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Test Case 4: %d\n", reversePairs(nums4, size4)); // Expected: 1

    // Test Case 5
    int nums5[] = {-1, -2, -3};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    printf("Test Case 5: %d\n", reversePairs(nums5, size5)); // Expected: 3
    
    return 0;
}