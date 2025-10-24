#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void merge(long long* sums, int start, int mid, int end) {
    int size = end - start + 1;
    long long* temp = (long long*)malloc(size * sizeof(long long));
    if (temp == NULL) return; // Error handling

    int i = start, j = mid + 1, t = 0;
    while (i <= mid && j <= end) {
        if (sums[i] <= sums[j]) {
            temp[t++] = sums[i++];
        } else {
            temp[t++] = sums[j++];
        }
    }
    while (i <= mid) {
        temp[t++] = sums[i++];
    }
    while (j <= end) {
        temp[t++] = sums[j++];
    }
    
    memcpy(sums + start, temp, size * sizeof(long long));
    
    free(temp);
}

int countAndMerge(long long* sums, int start, int end, int lower, int upper) {
    if (start >= end) {
        return 0;
    }

    int mid = start + (end - start) / 2;
    int count = countAndMerge(sums, start, mid, lower, upper) + countAndMerge(sums, mid + 1, end, lower, upper);

    int k = start, l = start;
    for (int j = mid + 1; j <= end; j++) {
        while (k <= mid && sums[k] < sums[j] - upper) {
            k++;
        }
        while (l <= mid && sums[l] <= sums[j] - lower) {
            l++;
        }
        count += l - k;
    }
    
    merge(sums, start, mid, end);
    
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    if (nums == NULL || numsSize == 0) {
        return 0;
    }
    
    long long* prefixSums = (long long*)malloc((numsSize + 1) * sizeof(long long));
    if (prefixSums == NULL) return -1; // Error handling
    
    prefixSums[0] = 0;
    for (int i = 0; i < numsSize; i++) {
        prefixSums[i + 1] = prefixSums[i] + nums[i];
    }
    
    int count = countAndMerge(prefixSums, 0, numsSize, lower, upper);
    
    free(prefixSums);
    return count;
}


void run_test_case(const char* name, int* nums, int numsSize, int lower, int upper) {
    int result = countRangeSum(nums, numsSize, lower, upper);
    printf("%s: %d\n", name, result);
}

int main() {
    // Test case 1
    int nums1[] = {-2, 5, -1};
    run_test_case("Test Case 1", nums1, 3, -2, 2);

    // Test case 2
    int nums2[] = {0};
    run_test_case("Test Case 2", nums2, 1, 0, 0);
    
    // Test case 3
    int nums3[] = {2147483647, -2147483648, -1, 0};
    run_test_case("Test Case 3", nums3, 4, -1, 0);

    // Test case 4
    int nums4[] = {0, 0, 0};
    run_test_case("Test Case 4", nums4, 3, 0, 0);

    // Test case 5
    int nums5[] = {-1, 1};
    run_test_case("Test Case 5", nums5, 2, 0, 0);
    
    return 0;
}