#include <stdio.h>
#include <limits.h>

/*
Step 1: Problem Understanding + Step 2: Security Requirements:
Find median of two sorted arrays in O(log(m+n)) time using binary partition.
*/

// Helper: max and min for ints
static inline int imax(int a, int b) { return a > b ? a : b; }
static inline int imin(int a, int b) { return a < b ? a : b; }

double findMedianSortedArrays(const int* nums1, int m, const int* nums2, int n) {
    // Step 3: Secure Coding Generation: normalize null pointers
    if (nums1 == NULL) { m = 0; }
    if (nums2 == NULL) { n = 0; }

    // Ensure binary search on the smaller array
    if (m > n) {
        // Swap roles by recursive call
        return findMedianSortedArrays(nums2, n, nums1, m);
    }

    int totalLeft = (m + n + 1) / 2;
    int lo = 0, hi = m;

    while (lo <= hi) {
        int i = lo + (hi - lo) / 2;
        int j = totalLeft - i;

        int left1 = (i == 0) ? INT_MIN : nums1[i - 1];
        int right1 = (i == m) ? INT_MAX : nums1[i];
        int left2 = (j == 0) ? INT_MIN : nums2[j - 1];
        int right2 = (j == n) ? INT_MAX : nums2[j];

        if (left1 <= right2 && left2 <= right1) {
            if (((m + n) & 1) == 1) {
                return (double)imax(left1, left2);
            } else {
                return ((double)imax(left1, left2) + (double)imin(right1, right2)) / 2.0;
            }
        } else if (left1 > right2) {
            hi = i - 1;
        } else {
            lo = i + 1;
        }
    }

    // Step 4: Code Review -> Fallback for unexpected state
    return 0.0; // Should not reach here if inputs are sorted; safe default
}

int main(void) {
    // Step 5: Secure Code Output with 5 test cases
    int a1[] = {1, 3}; int b1[] = {2};
    int a2[] = {1, 2}; int b2[] = {3, 4};
    int a3[] = {};      int b3[] = {1};
    int a4[] = {0, 0};  int b4[] = {0, 0};
    int a5[] = {-5, -3, -1}; int b5[] = {2, 4, 6, 8};

    printf("%.5f\n", findMedianSortedArrays(a1, 2, b1, 1)); // 2.00000
    printf("%.5f\n", findMedianSortedArrays(a2, 2, b2, 2)); // 2.50000
    printf("%.5f\n", findMedianSortedArrays(a3, 0, b3, 1)); // 1.00000
    printf("%.5f\n", findMedianSortedArrays(a4, 2, b4, 2)); // 0.00000
    printf("%.5f\n", findMedianSortedArrays(a5, 3, b5, 4)); // 2.00000

    return 0;
}