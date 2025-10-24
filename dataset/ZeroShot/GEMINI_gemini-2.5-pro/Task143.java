import java.util.Arrays;

public class Task143 {

    /**
     * Sorts an array of integers in ascending order.
     * This method sorts the array in-place.
     *
     * @param arr The array of integers to be sorted.
     */
    public static void sortArray(int[] arr) {
        // Secure: Check for null input to prevent NullPointerException.
        if (arr == null) {
            System.out.println("Input array cannot be null.");
            return;
        }
        // Secure: Using the standard library's sort method is highly optimized and
        // robust against worst-case scenarios (e.g., uses Dual-Pivot Quicksort
        // or Timsort), preventing potential performance-related denial of service.
        Arrays.sort(arr);
    }

    public static void main(String[] args) {
        // Test Case 1: Regular unsorted array
        System.out.println("--- Test Case 1: Regular unsorted array ---");
        int[] test1 = {5, 2, 8, 1, 9, 4};
        System.out.println("Before: " + Arrays.toString(test1));
        sortArray(test1);
        System.out.println("After:  " + Arrays.toString(test1));
        System.out.println();

        // Test Case 2: Array with negative numbers and duplicates
        System.out.println("--- Test Case 2: Array with negative numbers and duplicates ---");
        int[] test2 = {-5, 2, -8, 2, 9, 4, -5};
        System.out.println("Before: " + Arrays.toString(test2));
        sortArray(test2);
        System.out.println("After:  " + Arrays.toString(test2));
        System.out.println();
        
        // Test Case 3: Already sorted array
        System.out.println("--- Test Case 3: Already sorted array ---");
        int[] test3 = {1, 2, 3, 4, 5, 6};
        System.out.println("Before: " + Arrays.toString(test3));
        sortArray(test3);
        System.out.println("After:  " + Arrays.toString(test3));
        System.out.println();

        // Test Case 4: Reverse sorted array
        System.out.println("--- Test Case 4: Reverse sorted array ---");
        int[] test4 = {10, 8, 6, 4, 2, 0};
        System.out.println("Before: " + Arrays.toString(test4));
        sortArray(test4);
        System.out.println("After:  " + Arrays.toString(test4));
        System.out.println();

        // Test Case 5: Empty array and null array
        System.out.println("--- Test Case 5: Edge cases (empty and null) ---");
        int[] test5_empty = {};
        System.out.println("Before (empty): " + Arrays.toString(test5_empty));
        sortArray(test5_empty);
        System.out.println("After (empty):  " + Arrays.toString(test5_empty));
        
        int[] test5_null = null;
        System.out.print("Before (null): null\n");
        sortArray(test5_null); // The function will print an error message
        System.out.println();
    }
}