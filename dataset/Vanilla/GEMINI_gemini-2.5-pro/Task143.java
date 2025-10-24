import java.util.Arrays;

public class Task143 {

    /**
     * Sorts an array of integers in ascending order. The sort is done in-place.
     *
     * @param arr The array of integers to sort.
     * @return The same array, now sorted.
     */
    public static int[] sortArray(int[] arr) {
        if (arr != null) {
            Arrays.sort(arr);
        }
        return arr;
    }

    public static void main(String[] args) {
        // Test Case 1: Standard unsorted array
        int[] arr1 = {5, 2, 8, 1, 9};
        System.out.println("Test Case 1");
        System.out.println("Original: " + Arrays.toString(arr1));
        sortArray(arr1);
        System.out.println("Sorted:   " + Arrays.toString(arr1));
        System.out.println();

        // Test Case 2: Reverse-sorted array
        int[] arr2 = {9, 8, 7, 6, 5, 4, 3, 2, 1};
        System.out.println("Test Case 2");
        System.out.println("Original: " + Arrays.toString(arr2));
        sortArray(arr2);
        System.out.println("Sorted:   " + Arrays.toString(arr2));
        System.out.println();

        // Test Case 3: Already sorted array
        int[] arr3 = {1, 2, 3, 4, 5};
        System.out.println("Test Case 3");
        System.out.println("Original: " + Arrays.toString(arr3));
        sortArray(arr3);
        System.out.println("Sorted:   " + Arrays.toString(arr3));
        System.out.println();

        // Test Case 4: Array with duplicate elements
        int[] arr4 = {4, 2, 7, 2, 5, 4};
        System.out.println("Test Case 4");
        System.out.println("Original: " + Arrays.toString(arr4));
        sortArray(arr4);
        System.out.println("Sorted:   " + Arrays.toString(arr4));
        System.out.println();

        // Test Case 5: Array with negative numbers and zero
        int[] arr5 = {-5, 0, 10, -2, 3};
        System.out.println("Test Case 5");
        System.out.println("Original: " + Arrays.toString(arr5));
        sortArray(arr5);
        System.out.println("Sorted:   " + Arrays.toString(arr5));
        System.out.println();
    }
}