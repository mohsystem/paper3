import java.util.Arrays;

public class Task185 {
    public double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Ensure nums1 is the smaller array to optimize binary search range
        if (nums1.length > nums2.length) {
            return findMedianSortedArrays(nums2, nums1);
        }

        int m = nums1.length;
        int n = nums2.length;
        int low = 0;
        int high = m;

        while (low <= high) {
            int partitionX = low + (high - low) / 2;
            int partitionY = (m + n + 1) / 2 - partitionX;

            int maxLeftX = (partitionX == 0) ? Integer.MIN_VALUE : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? Integer.MAX_VALUE : nums1[partitionX];

            int maxLeftY = (partitionY == 0) ? Integer.MIN_VALUE : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? Integer.MAX_VALUE : nums2[partitionY];

            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                // Correct partition found
                if ((m + n) % 2 == 0) {
                    // Even number of elements: median is avg of max of left parts and min of right parts
                    return ((double) Math.max(maxLeftX, maxLeftY) + Math.min(minRightX, minRightY)) / 2;
                } else {
                    // Odd number of elements: median is max of left parts
                    return (double) Math.max(maxLeftX, maxLeftY);
                }
            } else if (maxLeftX > minRightY) {
                // partitionX is too large, move to the left in nums1
                high = partitionX - 1;
            } else { // maxLeftY > minRightX
                // partitionX is too small, move to the right in nums1
                low = partitionX + 1;
            }
        }
        
        // This should not be reached if the input arrays are sorted.
        throw new IllegalArgumentException("Input arrays are not sorted.");
    }

    public static void main(String[] args) {
        Task185 solution = new Task185();

        // Test Case 1
        int[] nums1_1 = {1, 3};
        int[] nums1_2 = {2};
        System.out.printf("Test Case 1: %.5f\n", solution.findMedianSortedArrays(nums1_1, nums1_2)); // Expected: 2.00000

        // Test Case 2
        int[] nums2_1 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.printf("Test Case 2: %.5f\n", solution.findMedianSortedArrays(nums2_1, nums2_2)); // Expected: 2.50000

        // Test Case 3
        int[] nums3_1 = {0, 0};
        int[] nums3_2 = {0, 0};
        System.out.printf("Test Case 3: %.5f\n", solution.findMedianSortedArrays(nums3_1, nums3_2)); // Expected: 0.00000

        // Test Case 4
        int[] nums4_1 = {};
        int[] nums4_2 = {1};
        System.out.printf("Test Case 4: %.5f\n", solution.findMedianSortedArrays(nums4_1, nums4_2)); // Expected: 1.00000

        // Test Case 5
        int[] nums5_1 = {2};
        int[] nums5_2 = {};
        System.out.printf("Test Case 5: %.5f\n", solution.findMedianSortedArrays(nums5_1, nums5_2)); // Expected: 2.00000
    }
}