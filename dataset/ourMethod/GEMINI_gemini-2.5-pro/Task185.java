public class Task185 {

    public double findMedianSortedArrays(int[] nums1, int[] nums2) {
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

            int maxLeftX = (partitionX == 0) ? Integer.MIN_VALUE : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? Integer.MAX_VALUE : nums1[partitionX];

            int maxLeftY = (partitionY == 0) ? Integer.MIN_VALUE : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? Integer.MAX_VALUE : nums2[partitionY];

            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                if ((m + n) % 2 == 0) {
                    return ((double) Math.max(maxLeftX, maxLeftY) + Math.min(minRightX, minRightY)) / 2.0;
                } else {
                    return (double) Math.max(maxLeftX, maxLeftY);
                }
            } else if (maxLeftX > minRightY) {
                high = partitionX - 1;
            } else {
                low = partitionX + 1;
            }
        }
        
        throw new IllegalArgumentException("Input arrays are invalid.");
    }

    public static void main(String[] args) {
        Task185 solver = new Task185();

        // Test Case 1
        int[] nums1_1 = {1, 3};
        int[] nums2_1 = {2};
        System.out.println("Test Case 1: " + solver.findMedianSortedArrays(nums1_1, nums2_1)); // Expected: 2.0

        // Test Case 2
        int[] nums1_2 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.println("Test Case 2: " + solver.findMedianSortedArrays(nums1_2, nums2_2)); // Expected: 2.5

        // Test Case 3
        int[] nums1_3 = {0, 0};
        int[] nums2_3 = {0, 0};
        System.out.println("Test Case 3: " + solver.findMedianSortedArrays(nums1_3, nums2_3)); // Expected: 0.0

        // Test Case 4
        int[] nums1_4 = {};
        int[] nums2_4 = {1};
        System.out.println("Test Case 4: " + solver.findMedianSortedArrays(nums1_4, nums2_4)); // Expected: 1.0

        // Test Case 5
        int[] nums1_5 = {2};
        int[] nums2_5 = {};
        System.out.println("Test Case 5: " + solver.findMedianSortedArrays(nums1_5, nums2_5)); // Expected: 2.0
    }
}