#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(int* nums, int start, int mid, int end) {
    int size = end - start + 1;
    int* temp = (int*)malloc(size * sizeof(int));
    if (temp == NULL) {
        // Handle memory allocation failure
        return;
    }

    int i = start, j = mid + 1, k = 0;
    while (i <= mid && j <= end) {
        if (nums[i] <= nums[j]) {
            temp[k++] = nums[i++];
        } else {
            temp[k++] = nums[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = nums[i++];
    }

    while (j <= end) {
        temp[k++] = nums[j++];
    }

    for (int l = 0; l < size; l++) {
        nums[start + l] = temp[l];
    }
    
    free(temp);
}

int mergeSortAndCount(int* nums, int start, int end) {
    if (start >= end) {
        return 0;
    }

    int mid = start + (end - start) / 2;
    int count = mergeSortAndCount(nums, start, mid);
    count += mergeSortAndCount(nums, mid + 1, end);

    int j = mid + 1;
    for (int i = start; i <= mid; i++) {
        while (j <= end && (long long)nums[i] > 2LL * nums[j]) {
            j++;
        }
        count += j - (mid + 1);
    }
    
    merge(nums, start, mid, end);

    return count;
}

int reversePairs(int* nums, int numsSize) {
    if (nums == NULL || numsSize < 2) {
        return 0;
    }
    // Create a mutable copy of the input array
    int* nums_copy = (int*)malloc(numsSize * sizeof(int));
    if (!nums_copy) return -1; // Indicate error
    memcpy(nums_copy, nums, numsSize * sizeof(int));
    
    int count = mergeSortAndCount(nums_copy, 0, numsSize - 1);
    
    free(nums_copy);
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
    int nums3[] = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Test Case 3: %d\n", reversePairs(nums3, size3));

    // Test Case 4
    int nums4[] = {-5, -5};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Test Case 4: %d\n", reversePairs(nums4, size4));
    
    // Test Case 5
    int* nums5 = NULL;
    int size5 = 0;
    printf("Test Case 5: %d\n", reversePairs(nums5, size5));

    return 0;
}