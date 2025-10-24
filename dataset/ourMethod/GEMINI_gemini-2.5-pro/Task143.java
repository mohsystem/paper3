import java.util.Arrays;

public class Task143 {

    /**
     * Sorts an array of integers in ascending order.
     * The sorting is done in-place.
     *
     * @param arr The array of integers to be sorted. Can be null or empty.
     */
    public static void sortArray(int[] arr) {
        // Rule #3: Validate input. Check for null to prevent NullPointerException.
        // An empty array is valid and requires no action.
        if (arr == null) {
            return;
        }
        // java.util.Arrays.sort() is a highly optimized and safe sorting algorithm.
        // It handles array boundaries correctly, adhering to Rule #4.
        Arrays.sort(arr);
    }

    /**
     * Main method for testing the sortArray function.
     *
     * @param args Command line arguments (not used).
     */
    public static void main(String[] args) {
        // Test cases
        int[][] testCases = {
            {},                                 // Test Case 1: Empty array
            {5, -2, 9, 1, 5, 6},                // Test Case 2: Mixed positive and negative numbers
            {1},                                // Test Case 3: Single element array
            {9, 8, 7, 6, 5, 4, 3, 2, 1},        // Test Case 4: Reverse sorted array
            {1, 2, 3, 4, 5, 6, 7, 8, 9}         // Test Case 5: Already sorted array
        };
        
        for (int i = 0; i < testCases.length; i++) {
            int[] testArray = testCases[i];
            System.out.println("Test Case " + (i + 1) + ":");
            System.out.println("Original array: " + Arrays.toString(testArray));
            sortArray(testArray);
            System.out.println("Sorted array:   " + Arrays.toString(testArray));
            System.out.println();
        }
    }
}