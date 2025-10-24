
import java.util.Arrays;

public class Task146 {
    /**
     * Finds the missing number in an array of unique integers from 1 to n.
     * Uses sum formula to avoid overflow issues.
     * 
     * @param arr Array of unique integers with one number missing
     * @return The missing number, or -1 if input is invalid
     */
    public static int findMissingNumber(int[] arr) {
        // Validate input
        if (arr == null || arr.length == 0) {
            return -1;
        }
        
        // Array length + 1 gives us n (since one number is missing)
        long n = (long) arr.length + 1;
        
        // Validate n is within reasonable bounds to prevent overflow
        if (n > Integer.MAX_VALUE || n < 1) {
            return -1;
        }
        
        // Calculate expected sum using formula: n * (n + 1) / 2
        long expectedSum = (n * (n + 1)) / 2;
        
        // Calculate actual sum of array elements
        long actualSum = 0;
        for (int num : arr) {
            // Validate each number is within range [1, n]
            if (num < 1 || num > n) {
                return -1;
            }
            actualSum += num;
        }
        
        // The difference is the missing number
        long missing = expectedSum - actualSum;
        
        // Validate result is within integer range
        if (missing < 1 || missing > n) {
            return -1;
        }
        
        return (int) missing;
    }
    
    public static void main(String[] args) {
        // Test case 1: Missing number is 5
        int[] test1 = {1, 2, 3, 4, 6, 7, 8, 9, 10};
        System.out.println("Test 1 - Array: " + Arrays.toString(test1));
        System.out.println("Missing number: " + findMissingNumber(test1));
        System.out.println();
        
        // Test case 2: Missing number is 1
        int[] test2 = {2, 3, 4, 5};
        System.out.println("Test 2 - Array: " + Arrays.toString(test2));
        System.out.println("Missing number: " + findMissingNumber(test2));
        System.out.println();
        
        // Test case 3: Missing number is 10
        int[] test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        System.out.println("Test 3 - Array: " + Arrays.toString(test3));
        System.out.println("Missing number: " + findMissingNumber(test3));
        System.out.println();
        
        // Test case 4: Small array, missing 2
        int[] test4 = {1};
        System.out.println("Test 4 - Array: " + Arrays.toString(test4));
        System.out.println("Missing number: " + findMissingNumber(test4));
        System.out.println();
        
        // Test case 5: Missing number is 3
        int[] test5 = {1, 2, 4, 5, 6};
        System.out.println("Test 5 - Array: " + Arrays.toString(test5));
        System.out.println("Missing number: " + findMissingNumber(test5));
    }
}
