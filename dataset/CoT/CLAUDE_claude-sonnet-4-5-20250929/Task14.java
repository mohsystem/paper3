
public class Task14 {
    public static int findEqualIndex(int[] arr) {
        // Input validation to prevent null pointer exceptions
        if (arr == null || arr.length == 0) {
            return -1;
        }
        
        // Calculate total sum to avoid repeated calculations
        long totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }
        
        // Track left sum as we iterate
        long leftSum = 0;
        
        // Iterate through array to find equilibrium index
        for (int i = 0; i < arr.length; i++) {
            // Right sum = total - left - current element
            long rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            // Update left sum for next iteration
            leftSum += arr[i];
        }
        
        return -1;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Test 1: " + findEqualIndex(test1)); // Expected: 3
        
        // Test case 2
        int[] test2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Test 2: " + findEqualIndex(test2)); // Expected: 1
        
        // Test case 3
        int[] test3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Test 3: " + findEqualIndex(test3)); // Expected: 0
        
        // Test case 4
        int[] test4 = {1, 2, 3};
        System.out.println("Test 4: " + findEqualIndex(test4)); // Expected: -1
        
        // Test case 5
        int[] test5 = {0, 0, 0, 0};
        System.out.println("Test 5: " + findEqualIndex(test5)); // Expected: 0
    }
}
