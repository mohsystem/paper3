import java.util.Arrays;

public class Task145 {

    /**
     * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
     *
     * @param arr The input array of integers.
     * @return The maximum sum of a contiguous subarray. Returns 0 if the array is null or empty.
     */
    public static long findMaxSubarraySum(int[] arr) {
        // Handle null or empty array case.
        if (arr == null || arr.length == 0) {
            return 0;
        }

        // Use long to prevent integer overflow with large sums.
        long maxSoFar = arr[0];
        long currentMax = arr[0];

        for (int i = 1; i < arr.length; i++) {
            // Decide whether to extend the existing subarray or start a new one.
            currentMax = Math.max(arr[i], currentMax + arr[i]);
            // Update the overall maximum sum found so far.
            maxSoFar = Math.max(maxSoFar, currentMax);
        }

        return maxSoFar;
    }

    public static void main(String[] args) {
        // Test Case 1: Mixed positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Array: " + Arrays.toString(test1));
        System.out.println("Maximum subarray sum: " + findMaxSubarraySum(test1)); // Expected: 6
        System.out.println();

        // Test Case 2: All positive numbers
        int[] test2 = {1, 2, 3, 4, 5};
        System.out.println("Array: " + Arrays.toString(test2));
        System.out.println("Maximum subarray sum: " + findMaxSubarraySum(test2)); // Expected: 15
        System.out.println();

        // Test Case 3: All negative numbers
        int[] test3 = {-2, -3, -4, -1, -2, -1, -5, -3};
        System.out.println("Array: " + Arrays.toString(test3));
        System.out.println("Maximum subarray sum: " + findMaxSubarraySum(test3)); // Expected: -1
        System.out.println();
        
        // Test Case 4: Single element array
        int[] test4 = {5};
        System.out.println("Array: " + Arrays.toString(test4));
        System.out.println("Maximum subarray sum: " + findMaxSubarraySum(test4)); // Expected: 5
        System.out.println();
        
        // Test Case 5: Empty array
        int[] test5 = {};
        System.out.println("Array: " + Arrays.toString(test5));
        System.out.println("Maximum subarray sum: " + findMaxSubarraySum(test5)); // Expected: 0
        System.out.println();
    }
}