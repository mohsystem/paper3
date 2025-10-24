import java.util.Arrays;

public class Task185 {

    public double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Ensure nums1 is the smaller array to optimize binary search
        if (nums1.length > nums2.length) {
            return findMedianSortedArrays(nums2, nums1);
        }

        int m = nums1.length;
        int n = nums2.length;
        int low = 0;
        int high = m;

        while (low <= high) {
            int partitionX = (low + high) / 2;
            int partitionY = (m + n + 1) / 2 - partitionX;

            // Get the four boundary elements for the partitions
            int maxX = (partitionX == 0) ? Integer.MIN_VALUE : nums1[partitionX - 1];
            int minX = (partitionX == m) ? Integer.MAX_VALUE : nums1[partitionX];

            int maxY = (partitionY == 0) ? Integer.MIN_VALUE : nums2[partitionY - 1];
            int minY = (partitionY == n) ? Integer.MAX_VALUE : nums2[partitionY];

            if (maxX <= minY && maxY <= minX) {
                // Correct partition found, calculate the median
                if ((m + n) % 2 == 0) {
                    // Even number of total elements
                    return (double)(Math.max(maxX, maxY) + Math.min(minX, minY)) / 2.0;
                } else {
                    // Odd number of total elements
                    return (double)Math.max(maxX, maxY);
                }
            } else if (maxX > minY) {
                // Partition in nums1 is too large, move to the left half
                high = partitionX - 1;
            } else {
                // Partition in nums1 is too small, move to the right half
                low = partitionX + 1;
            }
        }
        // This line should not be reached if the input arrays are sorted
        throw new IllegalArgumentException("Input arrays are not sorted.");
    }

    public static void main(String[] args) {
        Task185 solution = new Task185();

        // Test Case 1
        int[] nums1_1 = {1, 3};
        int[] nums2_1 = {2};
        System.out.println("Test Case 1: " + solution.findMedianSortedArrays(nums1_1, nums2_1));

        // Test Case 2
        int[] nums1_2 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.println("Test Case 2: " + solution.findMedianSortedArrays(nums1_2, nums2_2));

        // Test Case 3
        int[] nums1_3 = {0, 0};
        int[] nums2_3 = {0, 0};
        System.out.println("Test Case 3: " + solution.findMedianSortedArrays(nums1_3, nums2_3));

        // Test Case 4
        int[] nums1_4 = {};
        int[] nums2_4 = {1};
        System.out.println("Test Case 4: " + solution.findMedianSortedArrays(nums1_4, nums2_4));

        // Test Case 5
        int[] nums1_5 = {2};
        int[] nums2_5 = {};
        System.out.println("Test Case 5: " + solution.findMedianSortedArrays(nums1_5, nums2_5));
    }
}