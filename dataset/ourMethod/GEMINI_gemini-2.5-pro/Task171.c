#include <stdio.h>
#include <stdlib.h>

// Helper function to merge two sorted subarrays and count reverse pairs between them.
// The subarrays are nums[low..mid] and nums[mid+1..high].
int mergeAndCount(int* nums, int* temp, int low, int mid, int high) {
    int count = 0;
    
    // Count the reverse pairs where one element is in the left subarray
    // and the other is in the right subarray.
    int j = mid + 1;
    for (int i = low; i <= mid; i++) {
        while (j <= high && (long long)nums[i] > 2LL * nums[j]) {
            j++;
        }
        count += (j - (mid + 1));
    }

    // Merge the two sorted subarrays into the temp array.
    int i = low;
    j = mid + 1;
    int k = low;

    while ((i <= mid) && (j <= high)) {
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

    // Copy the merged elements from the temp array back to the original array.
    for (i = low; i <= high; i++) {
        nums[i] = temp[i];
    }

    return count;
}

// Recursive function that sorts the array and counts reverse pairs using a modified merge sort.
int mergeSort(int* nums, int* temp, int low, int high) {
    int count = 0;
    if (low < high) {
        int mid = low + (high - low) / 2;
        
        // Recursively count pairs in left and right halves.
        count += mergeSort(nums, temp, low, mid);
        count += mergeSort(nums, temp, mid + 1, high);
        
        // Count pairs that span across the two halves and merge them.
        count += mergeAndCount(nums, temp, low, mid, high);
    }
    return count;
}

// Main function to count reverse pairs.
// Note: This function modifies the input array `nums` by sorting it.
int reversePairs(int* nums, int numsSize) {
    if (nums == NULL || numsSize < 2) {
        return 0;
    }

    int* temp = (int*)malloc((size_t)numsSize * sizeof(int));
    if (temp == NULL) {
        // Handle memory allocation failure.
        return 0; 
    }

    int result = mergeSort(nums, temp, 0, numsSize - 1);
    
    free(temp);
    temp = NULL;

    return result;
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, 2, 3, 1};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    printf("Test Case 1: Input: [1, 3, 2, 3, 1]\n");
    printf("Output: %d\n\n", reversePairs(nums1, size1));

    // Test Case 2
    int nums2[] = {2, 4, 3, 5, 1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    printf("Test Case 2: Input: [2, 4, 3, 5, 1]\n");
    printf("Output: %d\n\n", reversePairs(nums2, size2));

    // Test Case 3: Large positive numbers
    int nums3[] = {2147483647, 2147483647, 2147483647, 2147483647, 2147483647};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    printf("Test Case 3: Input: [2147483647, ..., 2147483647]\n");
    printf("Output: %d\n\n", reversePairs(nums3, size3));
    
    // Test Case 4: Negative numbers
    int nums4[] = {-5, -5};
    int size4 = sizeof(nums4) / sizeof(nums4[0]);
    printf("Test Case 4: Input: [-5, -5]\n");
    printf("Output: %d\n\n", reversePairs(nums4, size4));

    // Test Case 5: Empty array
    int* nums5 = NULL;
    int size5 = 0;
    printf("Test Case 5: Input: []\n");
    printf("Output: %d\n\n", reversePairs(nums5, size5));
    
    return 0;
}