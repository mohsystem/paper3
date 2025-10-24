public class Task145 {

    /**
     * Finds the contiguous subarray with the maximum sum using Kadane's algorithm.
     *
     * @param a The input array of integers.
     * @return The maximum sum of a contiguous subarray.
     */
    public static int maxSubArraySum(int[] a) {
        if (a == null || a.length == 0) {
            return 0; // The sum of an empty subarray is 0.
        }

        int maxSoFar = a[0];
        int currentMax = a[0];

        for (int i = 1; i < a.length; i++) {
            currentMax = Math.max(a[i], currentMax + a[i]);
            maxSoFar = Math.max(maxSoFar, currentMax);
        }
        return maxSoFar;
    }

    public static void main(String[] args) {
        // Test Case 1
        int[] arr1 = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        System.out.println("Max sum for [-2, 1, -3, 4, -1, 2, 1, -5, 4] is: " + maxSubArraySum(arr1));

        // Test Case 2
        int[] arr2 = {1};
        System.out.println("Max sum for [1] is: " + maxSubArraySum(arr2));

        // Test Case 3
        int[] arr3 = {5, 4, -1, 7, 8};
        System.out.println("Max sum for [5, 4, -1, 7, 8] is: " + maxSubArraySum(arr3));

        // Test Case 4
        int[] arr4 = {-5, -1, -3};
        System.out.println("Max sum for [-5, -1, -3] is: " + maxSubArraySum(arr4));

        // Test Case 5
        int[] arr5 = {-2, -3, 4, -1, -2, 1, 5, -3};
        System.out.println("Max sum for [-2, -3, 4, -1, -2, 1, 5, -3] is: " + maxSubArraySum(arr5));
    }
}