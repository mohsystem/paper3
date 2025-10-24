
import java.util.Arrays;

public class Task145 {
    /**
     * Finds the contiguous subarray with maximum sum using Kadane's algorithm.
     * @param arr Input array of integers
     * @return Maximum sum of contiguous subarray
     * @throws IllegalArgumentException if array is null or empty
     */
    public static long maxSubarraySum(int[] arr) {
        // Input validation
        if (arr == null) {
            throw new IllegalArgumentException("Array cannot be null");
        }
        if (arr.length == 0) {
            throw new IllegalArgumentException("Array cannot be empty");
        }
        
        // Use long to prevent integer overflow
        long maxSoFar = (long) arr[0];
        long maxEndingHere = (long) arr[0];
        
        for (int i = 1; i < arr.length; i++) {
            maxEndingHere = Math.max((long) arr[i], maxEndingHere + (long) arr[i]);
            maxSoFar = Math.max(maxSoFar, maxEndingHere);
        }
        
        return maxSoFar;
    }
    
    public static void main(String[] args) {
        // Test case 1: Mixed positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Test 1: " + Arrays.toString(test1));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test1));
        System.out.println();
        
        // Test case 2: All positive numbers
        int[] test2 = {1, 2, 3, 4, 5};
        System.out.println("Test 2: " + Arrays.toString(test2));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test2));
        System.out.println();
        
        // Test case 3: All negative numbers
        int[] test3 = {-5, -2, -8, -1, -4};
        System.out.println("Test 3: " + Arrays.toString(test3));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test3));
        System.out.println();
        
        // Test case 4: Single element
        int[] test4 = {42};
        System.out.println("Test 4: " + Arrays.toString(test4));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test4));
        System.out.println();
        
        // Test case 5: Large numbers near overflow boundary
        int[] test5 = {1000000000, 1000000000, -500000000, 1000000000};
        System.out.println("Test 5: " + Arrays.toString(test5));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test5));
    }
}
