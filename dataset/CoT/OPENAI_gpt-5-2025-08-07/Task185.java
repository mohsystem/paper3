import java.util.*;

public class Task185 {
    // Step 1: Problem Understanding + Step 2: Security Requirements:
    // Function to find median of two sorted arrays in O(log(m+n)) time.
    // Handles null inputs safely by treating them as empty arrays.
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Step 3: Secure Coding Generation: input normalization
        if (nums1 == null) nums1 = new int[0];
        if (nums2 == null) nums2 = new int[0];

        int m = nums1.length, n = nums2.length;

        // Ensure binary search on smaller array to avoid out-of-bounds.
        if (m > n) return findMedianSortedArrays(nums2, nums1);

        int totalLeft = (m + n + 1) / 2;
        int lo = 0, hi = m;

        while (lo <= hi) {
            int i = lo + (hi - lo) / 2;
            int j = totalLeft - i;

            int left1 = (i == 0) ? Integer.MIN_VALUE : nums1[i - 1];
            int right1 = (i == m) ? Integer.MAX_VALUE : nums1[i];
            int left2 = (j == 0) ? Integer.MIN_VALUE : nums2[j - 1];
            int right2 = (j == n) ? Integer.MAX_VALUE : nums2[j];

            if (left1 <= right2 && left2 <= right1) {
                if (((m + n) & 1) == 1) {
                    return Math.max(left1, left2);
                } else {
                    return (Math.max(left1, left2) + Math.min(right1, right2)) / 2.0;
                }
            } else if (left1 > right2) {
                hi = i - 1;
            } else {
                lo = i + 1;
            }
        }

        // Step 4: Code Review -> Fallback for unexpected state
        throw new IllegalStateException("Invalid input: arrays must be sorted.");
    }

    public static void main(String[] args) {
        // Step 5: Secure Code Output with 5 test cases
        int[] a1 = {1, 3}, b1 = {2};
        int[] a2 = {1, 2}, b2 = {3, 4};
        int[] a3 = {}, b3 = {1};
        int[] a4 = {0, 0}, b4 = {0, 0};
        int[] a5 = {-5, -3, -1}, b5 = {2, 4, 6, 8};

        System.out.printf("%.5f%n", findMedianSortedArrays(a1, b1)); // 2.00000
        System.out.printf("%.5f%n", findMedianSortedArrays(a2, b2)); // 2.50000
        System.out.printf("%.5f%n", findMedianSortedArrays(a3, b3)); // 1.00000
        System.out.printf("%.5f%n", findMedianSortedArrays(a4, b4)); // 0.00000
        System.out.printf("%.5f%n", findMedianSortedArrays(a5, b5)); // 2.00000
    }
}