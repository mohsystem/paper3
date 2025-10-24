import java.util.Arrays;

public class Task147 {

    /**
     * Finds the kth largest element in an array of integers.
     * This method sorts the array and picks the element from the end.
     *
     * @param nums The input array of integers.
     * @param k    The position 'k' of the largest element to find (1-based index).
     * @return The integer value of the kth largest element.
     * @throws IllegalArgumentException if the array is null/empty or if k is out of bounds.
     */
    public static int findKthLargest(int[] nums, int k) {
        // Security: Validate inputs to prevent array out-of-bounds errors and handle invalid arguments.
        if (nums == null || nums.length == 0 || k <= 0 || k > nums.length) {
            throw new IllegalArgumentException("Invalid input: Array must not be empty and k must be between 1 and the array's length.");
        }

        // Sort the array in ascending order. Time complexity is O(N log N).
        Arrays.sort(nums);

        // The kth largest element is at index (length - k) in a 0-indexed array sorted in ascending order.
        return nums[nums.length - k];
    }

    public static void main(String[] args) {
        // Test Case 1: General case
        int[] nums1 = {3, 2, 1, 5, 6, 4};
        int k1 = 2;
        System.out.println("Test Case 1: Array = " + Arrays.toString(nums1) + ", k = " + k1);
        System.out.println("Result: " + findKthLargest(nums1, k1)); // Expected: 5

        // Test Case 2: With duplicates
        int[] nums2 = {3, 2, 3, 1, 2, 4, 5, 5, 6};
        int k2 = 4;
        System.out.println("\nTest Case 2: Array = " + Arrays.toString(nums2) + ", k = " + k2);
        System.out.println("Result: " + findKthLargest(nums2, k2)); // Expected: 4

        // Test Case 3: k = 1 (the largest element)
        int[] nums3 = {7, 6, 5, 4, 3, 2, 1};
        int k3 = 1;
        System.out.println("\nTest Case 3: Array = " + Arrays.toString(nums3) + ", k = " + k3);
        System.out.println("Result: " + findKthLargest(nums3, k3)); // Expected: 7

        // Test Case 4: k = n (the smallest element)
        int[] nums4 = {7, 6, 5, 4, 3, 2, 1};
        int k4 = 7;
        System.out.println("\nTest Case 4: Array = " + Arrays.toString(nums4) + ", k = " + k4);
        System.out.println("Result: " + findKthLargest(nums4, k4)); // Expected: 1

        // Test Case 5: Invalid k (k > length of array)
        int[] nums5 = {1, 2};
        int k5 = 3;
        System.out.println("\nTest Case 5: Array = " + Arrays.toString(nums5) + ", k = " + k5);
        try {
            findKthLargest(nums5, k5);
        } catch (IllegalArgumentException e) {
            System.out.println("Result: Caught expected exception -> " + e.getMessage());
        }
    }
}