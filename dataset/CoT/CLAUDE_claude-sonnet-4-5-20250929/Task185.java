
public class Task185 {
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Ensure nums1 is the smaller array for efficiency
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
            
            // Handle edge cases with Integer.MIN_VALUE and Integer.MAX_VALUE
            int maxLeftX = (partitionX == 0) ? Integer.MIN_VALUE : nums1[partitionX - 1];
            int minRightX = (partitionX == m) ? Integer.MAX_VALUE : nums1[partitionX];
            
            int maxLeftY = (partitionY == 0) ? Integer.MIN_VALUE : nums2[partitionY - 1];
            int minRightY = (partitionY == n) ? Integer.MAX_VALUE : nums2[partitionY];
            
            if (maxLeftX <= minRightY && maxLeftY <= minRightX) {
                // Found the correct partition
                if ((m + n) % 2 == 0) {
                    return (Math.max(maxLeftX, maxLeftY) + Math.min(minRightX, minRightY)) / 2.0;
                } else {
                    return Math.max(maxLeftX, maxLeftY);
                }
            } else if (maxLeftX > minRightY) {
                high = partitionX - 1;
            } else {
                low = partitionX + 1;
            }
        }
        
        throw new IllegalArgumentException("Input arrays are not sorted");
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1_1 = {1, 3};
        int[] nums2_1 = {2};
        System.out.println("Test 1: " + findMedianSortedArrays(nums1_1, nums2_1)); // Expected: 2.0
        
        // Test case 2
        int[] nums1_2 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.println("Test 2: " + findMedianSortedArrays(nums1_2, nums2_2)); // Expected: 2.5
        
        // Test case 3
        int[] nums1_3 = {};
        int[] nums2_3 = {1};
        System.out.println("Test 3: " + findMedianSortedArrays(nums1_3, nums2_3)); // Expected: 1.0
        
        // Test case 4
        int[] nums1_4 = {1, 2, 3, 4, 5};
        int[] nums2_4 = {6, 7, 8, 9, 10};
        System.out.println("Test 4: " + findMedianSortedArrays(nums1_4, nums2_4)); // Expected: 5.5
        
        // Test case 5
        int[] nums1_5 = {1, 3, 5};
        int[] nums2_5 = {2, 4, 6, 8};
        System.out.println("Test 5: " + findMedianSortedArrays(nums1_5, nums2_5)); // Expected: 4.0
    }
}
