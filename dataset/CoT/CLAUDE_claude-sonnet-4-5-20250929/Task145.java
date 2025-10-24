
import java.util.Arrays;

public class Task145 {
    /**
     * Finds the contiguous subarray with maximum sum using Kadane's algorithm\n     * @param arr input array of integers\n     * @return maximum sum of contiguous subarray\n     */\n    public static int maxSubarraySum(int[] arr) {\n        // Input validation\n        if (arr == null || arr.length == 0) {\n            throw new IllegalArgumentException("Array cannot be null or empty");\n        }\n        \n        int maxSum = arr[0];\n        int currentSum = arr[0];\n        \n        // Kadane's algorithm
        for (int i = 1; i < arr.length; i++) {
            currentSum = Math.max(arr[i], currentSum + arr[i]);
            maxSum = Math.max(maxSum, currentSum);
        }
        
        return maxSum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Mixed positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Test 1: " + Arrays.toString(test1));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test1));
        System.out.println();
        
        // Test case 2: All negative numbers
        int[] test2 = {-5, -2, -8, -1, -4};
        System.out.println("Test 2: " + Arrays.toString(test2));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test2));
        System.out.println();
        
        // Test case 3: All positive numbers
        int[] test3 = {1, 2, 3, 4, 5};
        System.out.println("Test 3: " + Arrays.toString(test3));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test3));
        System.out.println();
        
        // Test case 4: Single element
        int[] test4 = {10};
        System.out.println("Test 4: " + Arrays.toString(test4));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test4));
        System.out.println();
        
        // Test case 5: Large numbers with overflow consideration
        int[] test5 = {-1, -2, 5, -3, 4, 6, -2};
        System.out.println("Test 5: " + Arrays.toString(test5));
        System.out.println("Maximum subarray sum: " + maxSubarraySum(test5));
    }
}
