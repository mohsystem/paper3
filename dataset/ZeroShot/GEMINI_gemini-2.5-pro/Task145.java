public class Task145 {
    /**
     * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
     *
     * @param a The input array of integers.
     * @return The sum of the maximum contiguous subarray. Returns 0 if the array is null or empty.
     */
    public static int maxSubArraySum(int[] a) {
        // Secure: Handle null or empty array to prevent NullPointerException or errors.
        if (a == null || a.length == 0) {
            return 0;
        }

        // Using int, assuming the sum will not overflow. For larger sums, long should be used.
        int maxSoFar = a[0];
        int currentMax = a[0];

        // Iterate from the second element
        for (int i = 1; i < a.length; i++) {
            // The maximum subarray ending at index i is either the element itself
            // or the element plus the maximum subarray ending at the previous index.
            currentMax = Math.max(a[i], currentMax + a[i]);
            
            // Update the overall maximum sum found so far.
            maxSoFar = Math.max(maxSoFar, currentMax);
        }
        return maxSoFar;
    }

    public static void main(String[] args) {
        // 5 test cases
        int[][] testCases = {
            {-2, 1, -3, 4, -1, 2, 1, -5, 4}, // Expected: 6, Subarray: [4, -1, 2, 1]
            {1, 2, 3, 4, 5},                // Expected: 15, Subarray: [1, 2, 3, 4, 5]
            {-2, -3, -4, -1, -5},           // Expected: -1, Subarray: [-1]
            {5},                            // Expected: 5, Subarray: [5]
            {8, -19, 5, -4, 20}             // Expected: 21, Subarray: [5, -4, 20]
        };

        for (int i = 0; i < testCases.length; i++) {
            int[] arr = testCases[i];
            System.out.print("Test Case " + (i + 1) + ": Array = [");
            for (int j = 0; j < arr.length; j++) {
                System.out.print(arr[j] + (j == arr.length - 1 ? "" : ", "));
            }
            System.out.println("], Max Subarray Sum = " + maxSubArraySum(arr));
        }
    }
}