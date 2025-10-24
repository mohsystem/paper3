import java.util.Arrays;

public class Task185 {

    /**
     * Finds the median of two sorted arrays.
     * The overall run time complexity is O(log(min(m, n))).
     *
     * @param nums1 The first sorted array.
     * @param nums2 The second sorted array.
     * @return The median of the two sorted arrays.
     */
    public double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Ensure nums1 is the smaller array to optimize binary search to O(log(min(m,n)))
        if (nums1.length > nums2.length) {
            return findMedianSortedArrays(nums2, nums1);
        }

        int m = nums1.length;
        int n = nums2.length;
        int low = 0;
        int high = m;
        
        // The total number of elements in the left partition
        int halfLen = (m + n + 1) / 2;

        while (low <= high) {
            // partitionX is the number of elements from nums1 in the left partition
            int partitionX = low + (high - low) / 2; // Avoids potential overflow
            int partitionY = halfLen - partitionX;

            // Get the boundary elements for the partitions
            int maxLeftX = (partitionX == 0) ? Integer.MIN_VALUE : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? Integer.MAX_VALUE : nums1[partitionX];

            int maxLeftY = (partitionY == 0) ? Integer.MIN_VALUE : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? Integer.MAX_VALUE : nums2[partitionY];

            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                // Correct partition found, calculate the median
                if ((m + n) % 2 == 0) {
                    // Even number of total elements: median is avg of two middle elements
                    return (double) (Math.max(maxLeftX, maxLeftY) + Math.min(minRightX, minRightY)) / 2.0;
                } else {
                    // Odd number of total elements: median is the single middle element
                    return (double) Math.max(maxLeftX, maxLeftY);
                }
            } else if (maxLeftX > minRightY) {
                // partitionX is too large, move to the left in nums1
                high = partitionX - 1;
            } else {
                // partitionX is too small, move to the right in nums1
                low = partitionX + 1;
            }
        }
        
        // This line should not be reached if the input arrays are sorted
        throw new IllegalArgumentException("Input arrays are not sorted or are invalid.");
    }
    
    public static void main(String[] args) {
        Task185 solution = new Task185();

        // Test Case 1
        int[] nums1_1 = {1, 3};
        int[] nums2_1 = {2};
        System.out.printf("Test Case 1: %.5f\n", solution.findMedianSortedArrays(nums1_1, nums2_1));

        // Test Case 2
        int[] nums1_2 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.printf("Test Case 2: %.5f\n", solution.findMedianSortedArrays(nums1_2, nums2_2));

        // Test Case 3
        int[] nums1_3 = {0, 0};
        int[] nums2_3 = {0, 0};
        System.out.printf("Test Case 3: %.5f\n", solution.findMedianSortedArrays(nums1_3, nums2_3));

        // Test Case 4
        int[] nums1_4 = {};
        int[] nums2_4 = {1};
        System.out.printf("Test Case 4: %.5f\n", solution.findMedianSortedArrays(nums1_4, nums2_4));

        // Test Case 5
        int[] nums1_5 = {2};
        int[] nums2_5 = {};
        System.out.printf("Test Case 5: %.5f\n", solution.findMedianSortedArrays(nums1_5, nums2_5));
    }
}