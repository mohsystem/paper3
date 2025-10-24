public class Task27 {

    /**
     * Finds the outlier number in an array where all numbers are either even or odd, except for one.
     *
     * @param arr The input array of integers, which must have a length of at least 3.
     * @return The outlier integer.
     * @throws IllegalArgumentException if the array has fewer than 3 elements or if no outlier is found.
     */
    public static int findOutlier(int[] arr) {
        if (arr == null || arr.length < 3) {
            throw new IllegalArgumentException("Array must contain at least 3 elements.");
        }

        int evenCount = 0;
        // Check the parity of the first three elements to determine the majority.
        for (int i = 0; i < 3; i++) {
            if (arr[i] % 2 == 0) {
                evenCount++;
            }
        }

        boolean majorityIsEven = evenCount >= 2;

        // Iterate through the array to find the number with the minority parity.
        for (int num : arr) {
            if (majorityIsEven) {
                // If the majority is even, the outlier is odd.
                if (num % 2 != 0) {
                    return num;
                }
            } else {
                // If the majority is odd, the outlier is even.
                if (num % 2 == 0) {
                    return num;
                }
            }
        }
        
        // This part should be unreachable based on the problem description.
        throw new IllegalStateException("No outlier found, which violates problem constraints.");
    }

    public static void main(String[] args) {
        // Test Case 1: Outlier is odd
        int[] test1 = {2, 4, 0, 100, 4, 11, 2602, 36};
        System.out.println("Test 1 Result: " + findOutlier(test1));

        // Test Case 2: Outlier is even
        int[] test2 = {160, 3, 1719, 19, 11, 13, -21};
        System.out.println("Test 2 Result: " + findOutlier(test2));

        // Test Case 3: Minimal array, outlier is even
        int[] test3 = {1, 1, 2};
        System.out.println("Test 3 Result: " + findOutlier(test3));
        
        // Test Case 4: Negative numbers involved, outlier is odd
        int[] test4 = {2, 6, 8, -10, 3};
        System.out.println("Test 4 Result: " + findOutlier(test4));

        // Test Case 5: Edge values, outlier is odd (Integer.MAX_VALUE)
        int[] test5 = {Integer.MAX_VALUE, 0, 2};
        System.out.println("Test 5 Result: " + findOutlier(test5));
    }
}