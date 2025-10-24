public class Task163 {

    /**
     * Calculates the length of the longest consecutive-run in an array of numbers.
     * A consecutive-run is a list of adjacent, consecutive integers, either increasing or decreasing.
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

        int maxRun = 1;
        int currentRun = 1;
        // direction: 0 = undecided, 1 = increasing, -1 = decreasing
        int direction = 0;

        for (int i = 1; i < arr.length; i++) {
            if (arr[i] == arr[i - 1] + 1) { // Increasing
                if (direction == 1) {
                    currentRun++;
                } else {
                    currentRun = 2;
                    direction = 1;
                }
            } else if (arr[i] == arr[i - 1] - 1) { // Decreasing
                if (direction == -1) {
                    currentRun++;
                } else {
                    currentRun = 2;
                    direction = -1;
                }
            } else { // Run broken
                currentRun = 1;
                direction = 0;
            }
            if (currentRun > maxRun) {
                maxRun = currentRun;
            }
        }
        return maxRun;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("Test 1: longestRun([1, 2, 3, 5, 6, 7, 8, 9])");
        System.out.println("Expected: 5, Got: " + longestRun(arr1));

        // Test Case 2
        int[] arr2 = {1, 2, 3, 10, 11, 15};
        System.out.println("\nTest 2: longestRun([1, 2, 3, 10, 11, 15])");
        System.out.println("Expected: 3, Got: " + longestRun(arr2));

        // Test Case 3
        int[] arr3 = {5, 4, 2, 1};
        System.out.println("\nTest 3: longestRun([5, 4, 2, 1])");
        System.out.println("Expected: 2, Got: " + longestRun(arr3));

        // Test Case 4
        int[] arr4 = {3, 5, 7, 10, 15};
        System.out.println("\nTest 4: longestRun([3, 5, 7, 10, 15])");
        System.out.println("Expected: 1, Got: " + longestRun(arr4));

        // Test Case 5
        int[] arr5 = {1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1};
        System.out.println("\nTest 5: longestRun([1, 0, -1, -2, 5, 6, 5, 4, 3, 2, 1])");
        System.out.println("Expected: 6, Got: " + longestRun(arr5));
    }
}