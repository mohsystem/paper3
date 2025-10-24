
public class Task14 {
    public static int findEqualSumIndex(int[] arr) {
        if (arr == null || arr.length == 0) {
            return -1;
        }
        
        // Calculate total sum of array
        int totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }
        
        // Iterate through array and check if left sum equals right sum
        int leftSum = 0;
        for (int i = 0; i < arr.length; i++) {
            // Right sum is total sum minus left sum minus current element
            int rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            // Add current element to left sum for next iteration
            leftSum += arr[i];
        }
        
        return -1;
    }
    
    public static void main(String[] args) {
        // Test case 1
        int[] test1 = {1, 2, 3, 4, 3, 2, 1};
        System.out.println("Test 1: " + findEqualSumIndex(test1)); // Expected: 3
        
        // Test case 2
        int[] test2 = {1, 100, 50, -51, 1, 1};
        System.out.println("Test 2: " + findEqualSumIndex(test2)); // Expected: 1
        
        // Test case 3
        int[] test3 = {20, 10, -80, 10, 10, 15, 35};
        System.out.println("Test 3: " + findEqualSumIndex(test3)); // Expected: 0
        
        // Test case 4
        int[] test4 = {1, 2, 3};
        System.out.println("Test 4: " + findEqualSumIndex(test4)); // Expected: -1
        
        // Test case 5
        int[] test5 = {10};
        System.out.println("Test 5: " + findEqualSumIndex(test5)); // Expected: 0
    }
}
