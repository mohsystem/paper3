
public class Task145 {
    public static int maxSubarraySum(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        
        int maxSoFar = arr[0];
        int maxEndingHere = arr[0];
        
        for (int i = 1; i < arr.length; i++) {
            maxEndingHere = Math.max(arr[i], maxEndingHere + arr[i]);
            maxSoFar = Math.max(maxSoFar, maxEndingHere);
        }
        
        return maxSoFar;
    }
    
    public static void main(String[] args) {
        // Test case 1: Mix of positive and negative numbers
        int[] test1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Test 1: " + maxSubarraySum(test1)); // Expected: 6
        
        // Test case 2: All negative numbers
        int[] test2 = {-5, -2, -8, -1, -4};
        System.out.println("Test 2: " + maxSubarraySum(test2)); // Expected: -1
        
        // Test case 3: All positive numbers
        int[] test3 = {1, 2, 3, 4, 5};
        System.out.println("Test 3: " + maxSubarraySum(test3)); // Expected: 15
        
        // Test case 4: Single element
        int[] test4 = {10};
        System.out.println("Test 4: " + maxSubarraySum(test4)); // Expected: 10
        
        // Test case 5: Mix with zeros
        int[] test5 = {-2, 0, -1, 3, -2, 5, -3};
        System.out.println("Test 5: " + maxSubarraySum(test5)); // Expected: 6
    }
}
