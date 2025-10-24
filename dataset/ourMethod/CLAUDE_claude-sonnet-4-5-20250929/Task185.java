
public class Task185 {
    public static double findMedianSortedArrays(int[] nums1, int[] nums2) {
        // Input validation
        if (nums1 == null || nums2 == null) {
            throw new IllegalArgumentException("Input arrays must not be null");
        }
        
        if (nums1.length > 1000 || nums2.length > 1000) {
            throw new IllegalArgumentException("Array length must not exceed 1000");
        }
        
        if (nums1.length + nums2.length == 0) {
            throw new IllegalArgumentException("At least one array must be non-empty");
        }
        
        if (nums1.length + nums2.length > 2000) {
            throw new IllegalArgumentException("Combined length must not exceed 2000");
        }
        
        // Ensure nums1 is the smaller array
        if (nums1.length > nums2.length) {
            int[] temp = nums1;
            nums1 = nums2;
            nums2 = temp;
        }
        
        int m = nums1.length;
        int n = nums2.length;
        int low = 0;
        int high = m;
        
        while (low <= high) {
            int partition1 = (low + high) / 2;
            int partition2 = (m + n + 1) / 2 - partition1;
            
            int maxLeft1 = (partition1 == 0) ? Integer.MIN_VALUE : nums1[partition1 - 1];
            int minRight1 = (partition1 == m) ? Integer.MAX_VALUE : nums1[partition1];
            
            int maxLeft2 = (partition2 == 0) ? Integer.MIN_VALUE : nums2[partition2 - 1];
            int minRight2 = (partition2 == n) ? Integer.MAX_VALUE : nums2[partition2];
            
            if (maxLeft1 <= minRight2 && maxLeft2 <= minRight1) {
                if ((m + n) % 2 == 0) {
                    return (Math.max(maxLeft1, maxLeft2) + Math.min(minRight1, minRight2)) / 2.0;
                } else {
                    return Math.max(maxLeft1, maxLeft2);
                }
            } else if (maxLeft1 > minRight2) {
                high = partition1 - 1;
            } else {
                low = partition1 + 1;
            }
        }
        
        throw new IllegalArgumentException("Input arrays are not sorted");
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] nums1_1 = {1, 3};
        int[] nums2_1 = {2};
        System.out.println("Test 1: " + findMedianSortedArrays(nums1_1, nums2_1));
        
        // Test case 2
        int[] nums1_2 = {1, 2};
        int[] nums2_2 = {3, 4};
        System.out.println("Test 2: " + findMedianSortedArrays(nums1_2, nums2_2));
        
        // Test case 3
        int[] nums1_3 = {};
        int[] nums2_3 = {1};
        System.out.println("Test 3: " + findMedianSortedArrays(nums1_3, nums2_3));
        
        // Test case 4
        int[] nums1_4 = {1, 2, 3, 4, 5};
        int[] nums2_4 = {6, 7, 8, 9, 10};
        System.out.println("Test 4: " + findMedianSortedArrays(nums1_4, nums2_4));
        
        // Test case 5
        int[] nums1_5 = {1};
        int[] nums2_5 = {2, 3, 4, 5};
        System.out.println("Test 5: " + findMedianSortedArrays(nums1_5, nums2_5));
    }
}
