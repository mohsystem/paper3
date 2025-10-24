import java.util.Arrays;

public class Task143 {

    /**
     * Sorts an array of integers in ascending order.
     *
     * @param arr The array of integers to be sorted.
     */
    public static void sortArray(int[] arr) {
        // Check for null or empty array to prevent NullPointerException.
        if (arr == null || arr.length == 0) {
            return;
        }
        Arrays.sort(arr);
    }

    public static void main(String[] args) {
        // Test Case 1: General case
        int[] arr1 = {5, 2, 8, 1, 9, 4};
        System.out.println("Test Case 1:");
        System.out.println("Original: " + Arrays.toString(arr1));
        sortArray(arr1);
        System.out.println("Sorted:   " + Arrays.toString(arr1));
        System.out.println();

        // Test Case 2: Already sorted
        int[] arr2 = {1, 2, 3, 4, 5, 6};
        System.out.println("Test Case 2:");
        System.out.println("Original: " + Arrays.toString(arr2));
        sortArray(arr2);
        System.out.println("Sorted:   " + Arrays.toString(arr2));
        System.out.println();

        // Test Case 3: Reverse sorted
        int[] arr3 = {10, 8, 6, 4, 2, 0};
        System.out.println("Test Case 3:");
        System.out.println("Original: " + Arrays.toString(arr3));
        sortArray(arr3);
        System.out.println("Sorted:   " + Arrays.toString(arr3));
        System.out.println();

        // Test Case 4: With duplicates and negative numbers
        int[] arr4 = {4, -1, 5, 2, 4, -1, 0};
        System.out.println("Test Case 4:");
        System.out.println("Original: " + Arrays.toString(arr4));
        sortArray(arr4);
        System.out.println("Sorted:   " + Arrays.toString(arr4));
        System.out.println();

        // Test Case 5: Empty array
        int[] arr5 = {};
        System.out.println("Test Case 5:");
        System.out.println("Original: " + Arrays.toString(arr5));
        sortArray(arr5);
        System.out.println("Sorted:   " + Arrays.toString(arr5));
        System.out.println();
    }
}