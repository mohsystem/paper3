import java.util.Arrays;

public class Task145 {

    /**
     * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
     *
     * @param nums The input array of integers.
     * @return The maximum sum of a contiguous subarray.
     * @throws IllegalArgumentException if the input array is null or empty.
     */
    public static int findMaxSubarraySum(int[] nums) {
        if (nums == null || nums.length == 0) {
            throw new IllegalArgumentException("Input array cannot be null or empty.");
        }

        int maxSoFar = nums[0];
        int currentMax = nums[0];

        for (int i = 1; i < nums.length; i++) {
            currentMax = Math.max(nums[i], currentMax + nums[i]);
            maxSoFar = Math.max(maxSoFar, currentMax);
        }

        return maxSoFar;
    }

    public static void main(String[] args) {
        // Test Case 1: Mixed positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Test Case 1: " + Arrays.toString(test1));
        System.out.println("Max sum: " + findMaxSubarraySum(test1)); // Expected: 6

        // Test Case 2: Mostly positive numbers
        int[] test2 = {5, 4, -1, 7, 8};
        System.out.println("\nTest Case 2: " + Arrays.toString(test2));
        System.out.println("Max sum: " + findMaxSubarraySum(test2)); // Expected: 23

        // Test Case 3: All negative numbers
        int[] test3 = {-2, -3, -1, -5};
        System.out.println("\nTest Case 3: " + Arrays.toString(test3));
        System.out.println("Max sum: " + findMaxSubarraySum(test3)); // Expected: -1

        // Test Case 4: Single element array
        int[] test4 = {5};
        System.out.println("\nTest Case 4: " + Arrays.toString(test4));
        System.out.println("Max sum: " + findMaxSubarraySum(test4)); // Expected: 5

        // Test Case 5: Another mixed array
        int[] test5 = {8, -19, 5, -4, 20};
        System.out.println("\nTest Case 5: " + Arrays.toString(test5));
        System.out.println("Max sum: " + findMaxSubarraySum(test5)); // Expected: 21
    }
}