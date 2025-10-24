import java.util.Arrays;

public class Task163 {

    /**
     * Calculates the length of the longest consecutive run in an array.
     * A consecutive run can be either increasing or decreasing.
     *
     * @param arr The input array of integers.
     * @return The length of the longest consecutive run.
     */
    public static int longestRun(int[] arr) {
        if (arr == null || arr.length == 0) {
            return 0;
        }
        if (arr.length == 1) {
            return 1;
        }

        int maxLength = 1;
        int currentIncreasing = 1;
        int currentDecreasing = 1;

        for (int i = 1; i < arr.length; i++) {
            if (arr[i] == arr[i - 1] + 1) {
                // Increasing run continues
                currentIncreasing++;
                // Reset decreasing run
                currentDecreasing = 1;
            } else if (arr[i] == arr[i - 1] - 1) {
                // Decreasing run continues
                currentDecreasing++;
                // Reset increasing run
                currentIncreasing = 1;
            } else {
                // Run is broken
                currentIncreasing = 1;
                currentDecreasing = 1;
            }
            maxLength = Math.max(maxLength, Math.max(currentIncreasing, currentDecreasing));
        }
        return maxLength;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("Test Case 1: " + Arrays.toString(arr1) + " -> " + longestRun(arr1)); // Expected: 5

        // Test Case 2
        int[] arr2 = {1, 2, 3, 10, 11, 15};
        System.out.println("Test Case 2: " + Arrays.toString(arr2) + " -> " + longestRun(arr2)); // Expected: 3

        // Test Case 3
        int[] arr3 = {5, 4, 2, 1};
        System.out.println("Test Case 3: " + Arrays.toString(arr3) + " -> " + longestRun(arr3)); // Expected: 2

        // Test Case 4
        int[] arr4 = {3, 5, 7, 10, 15};
        System.out.println("Test Case 4: " + Arrays.toString(arr4) + " -> " + longestRun(arr4)); // Expected: 1
        
        // Test Case 5
        int[] arr5 = {1, 0, -1, -2, -1, 0, 1, 2, 3};
        System.out.println("Test Case 5: " + Arrays.toString(arr5) + " -> " + longestRun(arr5)); // Expected: 6
    }
}