#include <stdio.h>
#include <limits.h>

// Helper for max of two integers
int int_max(int a, int b) {
    return a > b ? a : b;
}

// Helper for min of two integers
int int_min(int a, int b) {
    return a < b ? a : b;
}

double findMedianSortedArrays(int* nums1, int nums1Size, int* nums2, int nums2Size) {
    if (nums1Size > nums2Size) {
        // Recursive call with swapped arrays to ensure nums1 is smaller
        return findMedianSortedArrays(nums2, nums2Size, nums1, nums1Size);
    }

    int m = nums1Size;
    int n = nums2Size;
    int low = 0;
    int high = m;
    int halfLen = (m + n + 1) / 2;

    while (low <= high) {
        int partitionX = low + (high - low) / 2;
        int partitionY = halfLen - partitionX;

        int maxLeftX = (partitionX == 0) ? INT_MIN : nums1[partitionX - 1];
        int minRightX = (partitionX == m) ? INT_MAX : nums1[partitionX];

        int maxLeftY = (partitionY == 0) ? INT_MIN : nums2[partitionY - 1];
        int minRightY = (partitionY == n) ? INT_MAX : nums2[partitionY];
        
        if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
            if ((m + n) % 2 == 0) {
                return (double)(int_max(maxLeftX, maxLeftY) + int_min(minRightX, minRightY)) / 2.0;
            } else {
                return (double)int_max(maxLeftX, maxLeftY);
            }
        } else if (maxLeftX > minRightY) {
            high = partitionX - 1;
        } else {
            low = partitionX + 1;
        }
    }

    // Should not be reached for valid sorted arrays.
    return -1.0; 
}

void run_test_case(const char* name, int* nums1, int nums1Size, int* nums2, int nums2Size) {
    double result = findMedianSortedArrays(nums1, nums1Size, nums2, nums2Size);
    printf("%s: %.5f\n", name, result);
}

int main() {
    // Test Case 1
    int nums1_1[] = {1, 3};
    int nums2_1[] = {2};
    run_test_case("Test Case 1", nums1_1, 2, nums2_1, 1);

    // Test Case 2
    int nums1_2[] = {1, 2};
    int nums2_2[] = {3, 4};
    run_test_case("Test Case 2", nums1_2, 2, nums2_2, 2);

    // Test Case 3
    int nums1_3[] = {0, 0};
    int nums2_3[] = {0, 0};
    run_test_case("Test Case 3", nums1_3, 2, nums2_3, 2);
    
    // Test Case 4
    int* nums1_4 = NULL;
    int nums2_4[] = {1};
    run_test_case("Test Case 4", nums1_4, 0, nums2_4, 1);

    // Test Case 5
    int nums1_5[] = {2};
    int* nums2_5 = NULL;
    run_test_case("Test Case 5", nums1_5, 1, nums2_5, 0);

    return 0;
}