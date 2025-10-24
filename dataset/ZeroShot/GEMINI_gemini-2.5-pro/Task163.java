public class Task163 {

    /**
     * Finds the length of the longest consecutive run in an array.
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
        int currentIncLength = 1;
        int currentDecLength = 1;

        for (int i = 1; i < arr.length; i++) {
            // Check for an increasing run
            if (arr[i] == arr[i - 1] + 1) {
                currentIncLength++;
            } else {
                currentIncLength = 1;
            }

            // Check for a decreasing run
            if (arr[i] == arr[i - 1] - 1) {
                currentDecLength++;
            } else {
                currentDecLength = 1;
            }

            // Update the maximum length found so far
            maxLength = Math.max(maxLength, Math.max(currentIncLength, currentDecLength));
        }

        return maxLength;
    }

    public static void main(String[] args) {
        // Test Case 1: Long increasing run
        int[] test1 = {1, 2, 3, 5, 6, 7, 8, 9};
        System.out.println("Test 1: {1, 2, 3, 5, 6, 7, 8, 9} -> Expected: 5, Got: " + longestRun(test1));

        // Test Case 2: Long decreasing run
        int[] test2 = {9, 8, 7, 6, 3, 2, 1};
        System.out.println("Test 2: {9, 8, 7, 6, 3, 2, 1} -> Expected: 4, Got: " + longestRun(test2));

        // Test Case 3: Mixed runs
        int[] test3 = {1, 2, 1, 0, -1, 4, 5};
        System.out.println("Test 3: {1, 2, 1, 0, -1, 4, 5} -> Expected: 4, Got: " + longestRun(test3));

        // Test Case 4: No consecutive runs
        int[] test4 = {3, 5, 7, 10, 15};
        System.out.println("Test 4: {3, 5, 7, 10, 15} -> Expected: 1, Got: " + longestRun(test4));

        // Test Case 5: Empty array
        int[] test5 = {};
        System.out.println("Test 5: {} -> Expected: 0, Got: " + longestRun(test5));
    }
}