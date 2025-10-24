
public class Task145 {
    public static int maxSubarraySum(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        
        int maxSum = arr[0];
        int currentSum = arr[0];
        
        for (int i = 1; i < arr.length; i++) {
            currentSum = Math.max(arr[i], currentSum + arr[i]);
            maxSum = Math.max(maxSum, currentSum);
        }
        
        return maxSum;
    }
    
    public static void main(String[] args) {
        // Test case 1: Mixed positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Test 1: " + maxSubarraySum(test1)); // Expected: 6 (subarray [4,-1,2,1])
        
        // Test case 2: All negative numbers
        int[] test2 = {-5, -2, -8, -1, -4};
        System.out.println("Test 2: " + maxSubarraySum(test2)); // Expected: -1
        
        // Test case 3: All positive numbers
        int[] test3 = {1, 2, 3, 4, 5};
        System.out.println("Test 3: " + maxSubarraySum(test3)); // Expected: 15
        
        // Test case 4: Single element
        int[] test4 = {5};
        System.out.println("Test 4: " + maxSubarraySum(test4)); // Expected: 5
        
        // Test case 5: Mix with zeros
        int[] test5 = {-2, 0, -1, 3, -1, 2, -3, 4};
        System.out.println("Test 5: " + maxSubarraySum(test5)); // Expected: 5 (subarray [3,-1,2,-3,4])
    }
}
