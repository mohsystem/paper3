#include <stdio.h>
#include <stdlib.h>

int mergeAndCount(int* nums, int left, int mid, int right);
int mergeSort(int* nums, int left, int right);

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
    int count = mergeSort(nums, left, mid);
    count += mergeSort(nums, mid + 1, right);
    count += mergeAndCount(nums, left, mid, right);
    return count;
}

int mergeAndCount(int* nums, int left, int mid, int right) {
    int count = 0;
    int j = mid + 1;
    for (int i = left; i <= mid; i++) {
        while (j <= right && (long long)nums[i] > 2LL * nums[j]) {
            j++;
        }
        count += (j - (mid + 1));
    }

    int tempSize = right - left + 1;
    int* temp = (int*)malloc(tempSize * sizeof(int));
    if (temp == NULL) {
        exit(1);
    }
    
    int i = left, k = mid + 1, p = 0;
    while (i <= mid && k <= right) {
        if (nums[i] <= nums[k]) {
            temp[p++] = nums[i++];
        } else {
            temp[p++] = nums[k++];
        }
    }
    while (i <= mid) {
        temp[p++] = nums[i++];
    }
    while (k <= right) {
        temp[p++] = nums[k++];
    }
    for (int l = 0; l < tempSize; l++) {
        nums[left + l] = temp[l];
    }
    
    free(temp);
    return count;
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, 2, 3, 1};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Test Case 1: %d\n", reversePairs(nums1, size1));

    // Test Case 2
    int nums2[] = {2, 4, 3, 5, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("Test Case 2: %d\n", reversePairs(nums2, size2));

    // Test Case 3
    int nums3[] = {};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Test Case 3: %d\n", reversePairs(nums3, size3));

    // Test Case 4
    int nums4[] = {5, 4, 3, 2, 1};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Test Case 4: %d\n", reversePairs(nums4, size4));

    // Test Case 5
    int nums5[] = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    printf("Test Case 5: %d\n", reversePairs(nums5, size5));

    return 0;
}