import java.util.Arrays;

public class Task147 {

    public static int findKthLargest(int[] nums, int k) {
        // Sort the array in ascending order
        Arrays.sort(nums);
        // The kth largest element is at index n-k where n is the length of the array
        return nums[nums.length - k];
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] nums1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test Case 1: " + findKthLargest(nums1, k1));

        // Test Case 2
        int[] nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("Test Case 2: " + findKthLargest(nums2, k2));

        // Test Case 3
        int[] nums3 = {1};
        int k3 = 1;
        System.out.println("Test Case 3: " + findKthLargest(nums3, k3));

        // Test Case 4
        int[] nums4 = {7, 6, 5, 4, 3, 2, 1};
        int k4 = 7;
        System.out.println("Test Case 4: " + findKthLargest(nums4, k4));

        // Test Case 5
        int[] nums5 = {-1, -1, 0, 2};
        int k5 = 3;
        System.out.println("Test Case 5: " + findKthLargest(nums5, k5));
    }
}