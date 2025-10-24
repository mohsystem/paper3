
import java.util.Arrays;

public class Task143 {
    /**
     * Sorts an array of integers in ascending order
     * @param arr The input array to be sorted
     * @return A new sorted array (original array remains unchanged)
     */
    public static int[] sortArray(int[] arr) {
        // Input validation - prevent null pointer exceptions
        if (arr == null) {
            return new int[0];
        }
        
        // Create a copy to avoid modifying the original array
        int[] result = Arrays.copyOf(arr, arr.length);
        
        // Use built-in sort which is secure and efficient
        Arrays.sort(result);
        
        return result;
    }
    
    public static void main(String[] args) {
        // Test case 1: Normal array
        int[] test1 = {5, 2, 8, 1, 9};
        System.out.println("Test 1: " + Arrays.toString(sortArray(test1)));
        
        // Test case 2: Already sorted array
        int[] test2 = {1, 2, 3, 4, 5};
        System.out.println("Test 2: " + Arrays.toString(sortArray(test2)));
        
        // Test case 3: Reverse sorted array
        int[] test3 = {9, 7, 5, 3, 1};
        System.out.println("Test 3: " + Arrays.toString(sortArray(test3)));
        
        // Test case 4: Array with duplicates
        int[] test4 = {4, 2, 7, 2, 9, 4};
        System.out.println("Test 4: " + Arrays.toString(sortArray(test4)));
        
        // Test case 5: Empty array
        int[] test5 = {};
        System.out.println("Test 5: " + Arrays.toString(sortArray(test5)));
    }
}
