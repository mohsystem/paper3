
import java.util.Arrays;

public class Task146 {
    /**
     * Finds the missing number in an array containing unique integers from 1 to n
     * @param arr Array of integers with one missing number
     * @return The missing number
     */
    public static int findMissingNumber(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 1;
        }
        
        int n = arr.length + 1;
        // Using mathematical formula: sum of 1 to n = n*(n+1)/2
        long expectedSum = (long) n * (n + 1) / 2;
        long actualSum = 0;
        
        for (int num : arr) {
            actualSum += num;
        }
        
        return (int) (expectedSum - actualSum);
    }
    
    public static void main(String[] args) {
        // Test case 1: Missing number is 5
        int[] test1 = {1, 2, 3, 4, 6};
        System.out.println("Test 1 - Array: " + Arrays.toString(test1));
        System.out.println("Missing number: " + findMissingNumber(test1));
        System.out.println();
        
        // Test case 2: Missing number is 1
        int[] test2 = {2, 3, 4, 5, 6};
        System.out.println("Test 2 - Array: " + Arrays.toString(test2));
        System.out.println("Missing number: " + findMissingNumber(test2));
        System.out.println();
        
        // Test case 3: Missing number is 10
        int[] test3 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        System.out.println("Test 3 - Array: " + Arrays.toString(test3));
        System.out.println("Missing number: " + findMissingNumber(test3));
        System.out.println();
        
        // Test case 4: Missing number is 3
        int[] test4 = {1, 2, 4, 5, 6, 7};
        System.out.println("Test 4 - Array: " + Arrays.toString(test4));
        System.out.println("Missing number: " + findMissingNumber(test4));
        System.out.println();
        
        // Test case 5: Array with single element missing (n=2)
        int[] test5 = {2};
        System.out.println("Test 5 - Array: " + Arrays.toString(test5));
        System.out.println("Missing number: " + findMissingNumber(test5));
        System.out.println();
    }
}
